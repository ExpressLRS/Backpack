Import("env")
import os
import re
import io
import tempfile
import filecmp
import shutil
import fnmatch
import elrs_helpers

import gzip
from minify import (html_minifier, rcssmin, rjsmin)

def get_version(env):
    return '%s (%s)' % (env.get('GIT_VERSION'), env.get('GIT_SHA'))

def build_version(out, env):
    out.write('const char *VERSION = "%s";\n\n' % get_version(env))

def compress(data):
    """Compress data in one shot and return the compressed string.
    Optional argument is the compression level, in range of 0-9.
    """
    buf = io.BytesIO()
    with gzip.GzipFile(fileobj=buf, mode='wb', compresslevel=9, mtime=0.0) as f:
        f.write(data)
    return buf.getvalue()

def build_html(mainfile, var, out, env):
    with open('html/%s' % mainfile, 'r') as file:
        data = file.read()
    if mainfile.endswith('.html'):
        data = html_minifier.html_minify(data).replace('@VERSION@', get_version(env)).replace('@PLATFORM@', re.sub("_via_.*", "", env['PIOENV']))
    if mainfile.endswith('.css'):
        data = rcssmin.cssmin(data)
    if mainfile.endswith('.js'):
        data = rjsmin.jsmin(data)
    out.write('static const char PROGMEM %s[] = {\n' % var)
    out.write(','.join("0x{:02x}".format(c) for c in compress(data.encode('utf-8'))))
    out.write('\n};\n\n')

def build_common(env, mainfile):
    fd, path = tempfile.mkstemp()
    try:
        with os.fdopen(fd, 'w') as out:
            build_version(out, env)
            build_html(mainfile, "INDEX_HTML", out, env)
            build_html("scan.js", "SCAN_JS", out, env)
            build_html("mui.js", "MUI_JS", out, env)
            build_html("elrs.css", "ELRS_CSS", out, env)
            build_html("mui.css", "MUI_CSS", out, env)
            build_html("logo.svg", "LOGO_SVG", out, env)
    finally:
        if not os.path.exists("include/WebContent.h") or not filecmp.cmp(path, "include/WebContent.h"):
            shutil.copyfile(path, "include/WebContent.h")
        os.remove(path)

platform = env.get('PIOPLATFORM', '')

if fnmatch.filter(env['BUILD_FLAGS'], '*TARGET_VRX_BACKPACK*'):
    build_common(env, "vrx_index.html")
elif fnmatch.filter(env['BUILD_FLAGS'], '*TARGET_TX_BACKPACK*'):
    build_common(env, "txbp_index.html")
elif fnmatch.filter(env['BUILD_FLAGS'], '*TARGET_TIMER_BACKPACK*'):
    build_common(env, "timer_index.html")
