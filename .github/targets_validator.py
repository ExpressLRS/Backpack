import sys
import json
import glob
import argparse
import configparser
import re
from dataclasses import dataclass

DEVICE_TYPES = ['txbp', 'vrx', 'timer', 'aat']
PLATFORMS = ['esp8285', 'esp32', 'esp32-c3', 'esp32-s3']
UPLOAD_METHODS = ['uart', 'etx', 'passthru', 'wifi']
DEVICE_KEYS = ['product_name', 'firmware', 'platform', 'upload_methods', 'validation_ignores']

# PlatformIO board -> targets.json "platform" value
BOARD_PLATFORMS = {
    'esp8285': 'esp8285',
    'esp12e': 'esp8285',
    'esp32dev': 'esp32',
    'esp32-c3-devkitm-1': 'esp32-c3',
    'esp32-s3-devkitc-1': 'esp32-s3',
}


@dataclass
class Finding:
    severity: str       # 'error' or 'warning'
    target: str         # 'vendor.type.device', or the offending vendor/environment
    code: str | None    # ignorable via the device's "validation_ignores" when set
    message: str


def load_environments():
    """Parse targets/*.ini and return {env_name: platform} for every [env:...] section,
    resolving each env's platform through its 'extends' chain down to an
    env_common_* section with a 'board'."""
    ini = configparser.RawConfigParser(strict=False)
    for inifile in sorted(glob.glob('targets/*.ini')):
        ini.read(inifile)

    def resolve_platform(section, seen=()):
        if section in seen or not ini.has_section(section):
            return None
        if ini.has_option(section, 'board'):
            return BOARD_PLATFORMS.get(ini.get(section, 'board'))
        if ini.has_option(section, 'extends'):
            for parent in re.split(r'[,\s]+', ini.get(section, 'extends')):
                platform = resolve_platform(parent, seen + (section,))
                if platform is not None:
                    return platform
        return None

    environments = {}
    for section in ini.sections():
        if section.startswith('env:'):
            environments[section[4:]] = resolve_platform(section)
    return environments


class TargetsValidator:
    def __init__(self, targets, environments):
        self.targets = targets
        self.environments = environments
        self.findings = []
        self.ignores = {}  # target -> that device's "validation_ignores" set

    def error(self, target, message, code=None):
        self.findings.append(Finding('error', target, code, message))

    def warn(self, target, message, code=None):
        self.findings.append(Finding('warning', target, code, message))

    def validate(self):
        for vendor in self.targets:
            self.validate_vendor(vendor, self.targets[vendor])
        self.validate_unreferenced_environments()
        return self.findings

    def validate_vendor(self, vendor, types):
        if not re.match(r'^[a-z0-9_-]+$', vendor):
            self.error(vendor, f'vendor tag "{vendor}" can only include lowercase a-z, 0-9, and underscores and dashes')

        if 'name' not in types or not isinstance(types['name'], str) or not types['name']:
            self.error(vendor, f'vendor "{vendor}" must have a non-empty "name" child element')

        if not any(type in DEVICE_TYPES for type in types):
            self.error(vendor, f'vendor "{vendor}" must have at least one device type, one of {DEVICE_TYPES}')

        for type in types:
            if type not in DEVICE_TYPES + ['name']:
                self.error(vendor, f'invalid tag "{type}" in "{vendor}"')
            if type in DEVICE_TYPES:
                for device in types[type]:
                    self.validate_device(vendor, type, device, types[type][device])

    def validate_device(self, vendor, type, devname, device):
        target = f'{vendor}.{type}.{devname}'
        self.ignores[target] = set(device.get('validation_ignores', []))

        if not re.match(r'^[A-Za-z0-9_-]+$', devname):
            self.error(target, f'device tag "{devname}" can only include a-z, A-Z, 0-9, and underscores and dashes', 'invalid_device_tag')

        for key in device:
            if key not in DEVICE_KEYS:
                self.error(target, f'device "{target}" has an unknown child element "{key}"', 'unknown_device_key')

        if 'product_name' not in device or not isinstance(device['product_name'], str) or not device['product_name']:
            self.error(target, f'device "{target}" must have a non-empty "product_name" child element', 'missing_product_name')

        if 'upload_methods' not in device or not isinstance(device['upload_methods'], list) or not device['upload_methods']:
            self.error(target, f'device "{target}" must have a non-empty "upload_methods" list', 'missing_upload_methods')
        else:
            methods = device['upload_methods']
            for method in methods:
                if method not in UPLOAD_METHODS:
                    self.error(target, f'invalid upload method "{method}" for target "{target}", must be one of {UPLOAD_METHODS}', 'invalid_upload_method')
            if len(set(methods)) != len(methods):
                self.error(target, f'duplicate upload methods for target "{target}"', 'duplicate_upload_method')

        platform = device.get('platform')
        if platform not in PLATFORMS:
            self.error(target, f'device "{target}" must have a "platform" child element, one of {PLATFORMS}', 'invalid_platform')
            platform = None

        if 'firmware' not in device or not isinstance(device['firmware'], str):
            self.error(target, f'device "{target}" must have a "firmware" child element', 'missing_firmware')
        else:
            firmware = device['firmware']
            env_platform = self.environments.get(firmware + '_via_UART')
            if firmware + '_via_UART' not in self.environments:
                self.error(target, f'device "{target}" firmware "{firmware}" has no [env:{firmware}_via_UART] in targets/*.ini', 'invalid_firmware_file')
            elif env_platform is None:
                self.error(target, f'could not determine the platform of [env:{firmware}_via_UART] from targets/*.ini', 'unknown_env_platform')
            elif platform is not None and env_platform != platform:
                self.error(target, f'device "{target}" "firmware" and "platform" MUST match: firmware "{firmware}" is {env_platform}, not {platform}', 'firmware_platform_mismatch')

    def validate_unreferenced_environments(self):
        firmwares = set()
        for vendor in self.targets:
            for type in self.targets[vendor]:
                if type in DEVICE_TYPES:
                    for device in self.targets[vendor][type].values():
                        if isinstance(device.get('firmware'), str):
                            firmwares.add(device['firmware'])
        for env in sorted(self.environments):
            if not env.endswith('_via_UART'):
                continue
            firmware = env[:-len('_via_UART')]
            if firmware.startswith('DEBUG_'):
                continue
            if firmware not in firmwares:
                self.warn(env, f'[env:{env}] in targets/*.ini is not referenced by any target in hardware/targets.json')

    def report(self, warn_as_error=False):
        had_error = False
        for finding in self.findings:
            if finding.code is not None and finding.code in self.ignores.get(finding.target, ()):
                continue
            print(f'{finding.severity.upper()}: {finding.message}')
            if finding.severity == 'error' or warn_as_error:
                had_error = True
        return 1 if had_error else 0


def main():
    parser = argparse.ArgumentParser(description="Validate hardware/targets.json")
    parser.add_argument("--warn", "-w", action='store_true', default=False, help="Treat warnings as errors")
    args = parser.parse_args()

    with open('hardware/targets.json') as f:
        try:
            targets = json.load(f)
        except json.JSONDecodeError as e:
            print(f'ERROR: hardware/targets.json is not valid JSON: {e}')
            return 1

    environments = load_environments()
    if not environments:
        print('ERROR: no [env:...] sections found in targets/*.ini, run from the repository root')
        return 1

    validator = TargetsValidator(targets, environments)
    validator.validate()
    return validator.report(warn_as_error=args.warn)


if __name__ == '__main__':
    sys.exit(main())
