import subprocess
import os
import datetime

Import("env")

def get_version():
    version = os.environ.get("FIRMWARE_VERSION")
    if version:
        print(f"--- [VERSION] Found FIRMWARE_VERSION in ENV: {version}")
        return version

    try:
        version = subprocess.check_output(["git", "describe", "--tags", "--always", "--dirty"], 
                                       stderr=subprocess.STDOUT).strip().decode("utf-8")
        print(f"--- [VERSION] Detected version from GIT: {version}")
        return version
    except:
        pass

    fallback = "dev-" + datetime.datetime.now().strftime("%y%m%d")
    print(f"--- [VERSION] Falling back to: {fallback}")
    return fallback

firmware_version = get_version()

env.Append(CPPDEFINES=[
    ("FIRMWARE_VERSION", f'\\"{firmware_version}\\"')
])

print(f"--- [VERSION] Macro defined: -DFIRMWARE_VERSION='\"{firmware_version}\"'")
