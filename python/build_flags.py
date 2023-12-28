Import("env")
from genericpath import exists
import os
from random import randint
import sys
import hashlib
import fnmatch
import time
import re
import elrs_helpers

build_flags = env.get('BUILD_FLAGS', [])
json_flags = {}
UIDbytes = ""
define = ""
target_name = env.get('PIOENV', '').upper()

def print_error(error):
    time.sleep(1)
    sys.stdout.write("\n\n\033[47;31m%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n")
    sys.stdout.write("\033[47;31m!!!             ExpressLRS Warning Below             !!!\n")
    sys.stdout.write("\033[47;31m%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n")
    sys.stdout.write("\033[47;30m  %s \n" % error)
    sys.stdout.write("\033[47;31m%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n\n\n")
    sys.stdout.flush()
    time.sleep(3)
    raise Exception('!!! %s !!!' % error)


def dequote(str):
    if str[0] == '"' and str[-1] == '"':
        return str[1:-1]
    return str

def process_json_flag(define):
    parts = re.search("-D(.*)\s*=\s*(.*)$", define)
    if parts and define.startswith("-D"):
        if parts.group(1) == "MY_BINDING_PHRASE":
            json_flags['uid'] = [x for x in hashlib.md5(define.encode()).digest()[0:6]]
        if parts.group(1) == "HOME_WIFI_SSID":
            json_flags['wifi-ssid'] = dequote(parts.group(2))
        if parts.group(1) == "HOME_WIFI_PASSWORD":
            json_flags['wifi-password'] = dequote(parts.group(2))
        if parts.group(1) == "AUTO_WIFI_ON_INTERVAL":
            parts = re.search("-D(.*)\s*=\s*\"?([0-9]+).*\"?$", define)
            json_flags['wifi-on-interval'] = int(dequote(parts.group(2)))

def process_build_flag(define):
    if define.startswith("-D") or define.startswith("!-D"):
        if "MY_BINDING_PHRASE" in define:
            build_flags.append(define)
            bindingPhraseHash = hashlib.md5(define.encode()).digest()
            UIDbytes = ",".join(list(map(str, bindingPhraseHash))[0:6])
            define = "-DMY_UID=" + UIDbytes
            sys.stdout.write("\u001b[32mUID bytes: " + UIDbytes + "\n")
            sys.stdout.flush()
        if "HOME_WIFI_SSID=" in define:
            parts = re.search("(.*)=\w*\"(.*)\"$", define)
            if parts and parts.group(2):
                define = "-DHOME_WIFI_SSID=" + string_to_ascii(parts.group(2))
        if "HOME_WIFI_PASSWORD=" in define:
            parts = re.search("(.*)=\w*\"(.*)\"$", define)
            if parts and parts.group(2):
                define = "-DHOME_WIFI_PASSWORD=" + string_to_ascii(parts.group(2))
        if not define in build_flags:
            build_flags.append(define)

def parse_flags(path):
    global build_flags
    global json_flags
    try:
        with open(path, "r") as _f:
            for define in _f:
                define = define.strip()
                process_build_flag(define)
                process_json_flag(define)

    except IOError:
        print("File '%s' does not exist" % path)

def process_flags(path):
    global build_flags
    if not os.path.isfile(path):
        return
    parse_flags(path)

def condense_flags():
    global build_flags
    for line in build_flags:
        # Some lines have multiple flags so this will split them and remove them all
        for flag in re.findall("!-D\s*[^\s]+", line):
            build_flags = [x.replace(flag,"") for x in build_flags] # remove the removal flag
            build_flags = [x.replace(flag[1:],"") for x in build_flags] # remove the flag if it matches the removal flag
    build_flags = [x for x in build_flags if (x.strip() != "")] # remove any blank items

def version_to_env():
    ver = elrs_helpers.get_git_version()
    env.Append(GIT_SHA = ver['sha'], GIT_VERSION= ver['version'])

def string_to_ascii(str):
    return ",".join(["%s" % ord(char) for char in str])

def get_git_sha():
    return string_to_ascii(env.get('GIT_SHA'))

def get_version():
    return string_to_ascii(env.get('GIT_VERSION'))

process_flags("user_defines.txt")
process_flags("super_defines.txt") # allow secret super_defines to override user_defines
version_to_env()
build_flags.append("-DLATEST_COMMIT=" + get_git_sha())
build_flags.append("-DLATEST_VERSION=" + get_version())
build_flags.append("-DTARGET_NAME=" + re.sub("_VIA_.*", "", target_name))
condense_flags()

json_flags['flash-discriminator'] = randint(1,2**32-1)
json_flags['product-name'] = re.sub("_via_.*", "", env.get('PIOENV', ''))

env['OPTIONS_JSON'] = json_flags
env['BUILD_FLAGS'] = build_flags
sys.stdout.write("\nbuild flags: %s\n\n" % build_flags)

if fnmatch.filter(build_flags, '*PLATFORM_ESP32*'):
    sys.stdout.write("\u001b[32mBuilding for ESP32 Platform\n")
elif fnmatch.filter(build_flags, '*PLATFORM_STM32*'):
    sys.stdout.write("\u001b[32mBuilding for STM32 Platform\n")
elif fnmatch.filter(build_flags, '*PLATFORM_ESP8266*'):
    sys.stdout.write("\u001b[32mBuilding for ESP8266/ESP8285 Platform\n")
    if fnmatch.filter(build_flags, '-DAUTO_WIFI_ON_INTERVAL*'):
        sys.stdout.write("\u001b[32mAUTO_WIFI_ON_INTERVAL = ON\n")
    else:
        sys.stdout.write("\u001b[32mAUTO_WIFI_ON_INTERVAL = OFF\n")

sys.stdout.flush()
time.sleep(.5)

# Set upload_protovol = 'custom' for STM32 MCUs
#  otherwise firmware.bin is not generated
stm = env.get('PIOPLATFORM', '') in ['ststm32']
if stm:
    env['UPLOAD_PROTOCOL'] = 'custom'
