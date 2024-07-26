import subprocess, os

def do_upload(elrs_bin_target, upload_addr, isstm, env):
    bootloader_target = None
    app_start = 0 # eka bootloader offset

    cmd = ["curl", "--max-time", "60",
           "--retry", "2", "--retry-delay", "1",
           "-F", "data=@%s" % (elrs_bin_target,)]

    if  bootloader_target is not None and isstm:
        cmd_bootloader = ["curl", "--max-time", "60",
            "--retry", "2", "--retry-delay", "1",
            "-F", "data=@%s" % (bootloader_target,), "-F", "flash_address=0x0000"]

    if isstm:
        cmd += ["-F", "flash_address=0x%X" % (app_start,)]

    upload_port = env.get('UPLOAD_PORT', None)
    if upload_port is not None:
        upload_addr = [upload_port]

    for addr in upload_addr:
        addr = "http://%s/%s" % (addr, ['update', 'upload'][isstm])
        print(" ** UPLOADING TO: %s" % addr)
        try:
            if  bootloader_target is not None:
                print("** Flashing Bootloader...")
                print(cmd_bootloader,cmd)
                subprocess.check_call(cmd_bootloader + [addr])
                print("** Bootloader Flashed!")
                print()
            subprocess.check_call(cmd + [addr])
            print()
            print("** UPLOAD SUCCESS. Flashing in progress.")
            print("** Please wait for LED to resume blinking before disconnecting power")
            return
        except subprocess.CalledProcessError:
            print("FAILED!")

    raise Exception("WIFI upload FAILED!")

def on_upload(source, target, env):
    isstm = env.get('PIOPLATFORM', '') in ['ststm32']
    upload_addr = ['elrs_txbp', 'elrs_txbp.local']

    firmware_path = str(source[0])
    bin_path = os.path.dirname(firmware_path)
    elrs_bin_target = os.path.join(bin_path, 'firmware.elrs')
    if not os.path.exists(elrs_bin_target):
        elrs_bin_target = os.path.join(bin_path, 'firmware.bin.gz')
        if not os.path.exists(elrs_bin_target):
            elrs_bin_target = os.path.join(bin_path, 'firmware.bin')
            if not os.path.exists(elrs_bin_target):
                raise Exception("No valid binary found!")
    do_upload(elrs_bin_target, upload_addr, isstm, env)
