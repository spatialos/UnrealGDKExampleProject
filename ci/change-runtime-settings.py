# coding=utf-8

# Purpose:
#   change runtime settings for build android and ios package

import os
import sys
import configparser
from collections import OrderedDict

# We customize Python's configparser slightly to allow for the same key to appear multiple times. 
# This happens when Unreal wants to create an array of values and it prepends each occurence of the key with a +
class MultiOrderedDict(OrderedDict):
    def __setitem__(self, key, value):
        if isinstance(value, list) and key in self and key.startswith('+'):
            self[key].extend(value)
        else:
            super(MultiOrderedDict, self).__setitem__(key, value)

# Support dumplicate keys
class CustomInterpolation(configparser.BasicInterpolation):
    def before_write(self, parser, section, option, value):
        if option.startswith('+'):
            return "\n{}=".format(option).join(value.split('\n'))
        return value

# Remove dumplicate key's \t 
class CustomWriter:
    output_file = None
    def __init__(self, new_output_file):
        self.output_file = new_output_file
    def write(self, what):
        self.output_file.write(what.replace('\n\t','\n'))
        

# Modify runtime settings before cook
def change_runtime_settings(project_home):
    default_engine_ini = os.path.join(project_home, 'Game', 'Config', 'DefaultEngine.ini')
    config = configparser.ConfigParser(strict=False, dict_type=MultiOrderedDict, interpolation=CustomInterpolation())
    config.optionxform = lambda option: option  # preserve case for letters
    config.read(default_engine_ini)

    IOSRuntimeSettings = '/Script/IOSRuntimeSettings.IOSRuntimeSettings'
    if not config.has_section(IOSRuntimeSettings):
        config.add_section(IOSRuntimeSettings)
    
    # Modify iOS runtime settings
    AdditionalPlistData = '<key>CFBundleURLTypes</key><array><dict><key>CFBundleURLName</key><string></string><key>CFBundleTypeRole</key><string>Editor</string><key>CFBundleURLSchemes</key><array><string>firebase-game-loop</string></array></dict></array>'
    config[IOSRuntimeSettings]['AdditionalPlistData'] = AdditionalPlistData
    config[IOSRuntimeSettings]['BundleIdentifier'] = 'io.improbable.unrealgdkdemo'
    config[IOSRuntimeSettings]['MobileProvision'] = ''
    config[IOSRuntimeSettings]['BundleDisplayName'] = 'UnrealGDKExampleProject'
    config[IOSRuntimeSettings]['BundleName'] = 'unrealgdkexampleproject'
    config[IOSRuntimeSettings]['bSupportsITunesFileSharing'] = 'True'
    config[IOSRuntimeSettings]['bGeneratedSYMFile'] = 'True'
    config[IOSRuntimeSettings]['bGeneratedSYMBundle'] = 'True'
    config[IOSRuntimeSettings]['bGenerateCrashReportSymbols'] = 'True'
    config[IOSRuntimeSettings]['bEnableRemoteNotificationsSupport'] = 'False'
    config[IOSRuntimeSettings]['bEnableAdvertisingIdentifier'] = 'False'
    config[IOSRuntimeSettings]['bEnableCloudKitSupport'] = 'False'
    
    # Modify Android extra activity settings
    AndroidRuntimeSettings = '/Script/AndroidRuntimeSettings.AndroidRuntimeSettings'
    if not config.has_section(AndroidRuntimeSettings):
        config.add_section(AndroidRuntimeSettings)
    ExtraActivitySettings = '<intent-filter><action android:name="com.google.intent.action.TEST_LOOP"/><category android:name="android.intent.category.DEFAULT"/><data android:mimeType="application/javascript"/></intent-filter>'  
    config[AndroidRuntimeSettings]['ExtraActivitySettings'] = ExtraActivitySettings
    config[AndroidRuntimeSettings]['bSupportAdMob'] = 'False'
    config[AndroidRuntimeSettings]['bPackageDataInsideApk'] = 'True'
    with open(default_engine_ini,'w') as fw:
        config.write(CustomWriter(fw), space_around_delimiters=False)


if __name__ == "__main__":
    # Path to the UnrealGDKExampleProject folder.
    project_home = sys.argv[1]
    change_runtime_settings(project_home)
