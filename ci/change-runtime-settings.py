# coding=utf-8

# Purpose:
#   change runtime settings for build android and ios package

import os
import sys
import configparser
from collections import OrderedDict


class MultiOrderedDict(OrderedDict):
    def __setitem__(self, key, value):
        if isinstance(value, list) and key in self and key.startswith('+'):
            self[key].extend(value)
        else:
            super(MultiOrderedDict, self).__setitem__(key, value)

class CustomInterpolation(configparser.BasicInterpolation):
    def before_write(self, parser, section, option, value):
        if option.startswith('+'):
            return "\n{}=".format(option).join(value.split('\n'))
        return value

class CustomWriter:
    output_file = None
    def __init__(self, new_output_file):
        self.output_file = new_output_file
    def write(self, what):
        self.output_file.write(what.replace(" = ", "=", 1).replace('\n\t','\n'))
        

# modify runtime settings before cook
def change_runtime_settings(project_home):
    defaultengine = os.path.join(project_home, 'Game', 'Config', 'DefaultEngine.ini')
    config = configparser.RawConfigParser(strict=False, dict_type=MultiOrderedDict, interpolation=CustomInterpolation())
    config.optionxform = lambda option: option  # preserve case for letters
    config.read(defaultengine)

    IOSRuntimeSettings = '/Script/IOSRuntimeSettings.IOSRuntimeSettings'
    if not config.has_section(IOSRuntimeSettings):
        config.add_section(IOSRuntimeSettings)
    
    # for ios runtime settings
    AdditionalPlistData = '<key>CFBundleURLTypes</key><array><dict><key>CFBundleURLName</key><string></string><key>CFBundleTypeRole</key><string>Editor</string><key>CFBundleURLSchemes</key><array><string>firebase-game-loop</string></array></dict></array>'
    config[IOSRuntimeSettings]['AdditionalPlistData'] = AdditionalPlistData
    config[IOSRuntimeSettings]['BundleIdentifier'] = 'io.improbabl.unrealgdkdemo'
    config[IOSRuntimeSettings]['MobileProvision'] = ''
    config[IOSRuntimeSettings]['BundleDisplayName'] = 'UnrealGDK Shoote'
    config[IOSRuntimeSettings]['BundleName'] = 'unrealgdkshooter'
    config[IOSRuntimeSettings]['bSupportsITunesFileSharing'] = 'True'
    config[IOSRuntimeSettings]['bGeneratedSYMFile'] = 'True'
    config[IOSRuntimeSettings]['bGeneratedSYMBundle'] = 'True'
    config[IOSRuntimeSettings]['bGenerateCrashReportSymbols'] = 'True'
    config[IOSRuntimeSettings]['bEnableRemoteNotificationsSupport'] = 'False'
    config[IOSRuntimeSettings]['bEnableAdvertisingIdentifier'] = 'False'
    config[IOSRuntimeSettings]['bEnableCloudKitSupport'] = 'False'
    
    # for Android extra activity settings
    AndroidRuntimeSettings = '/Script/AndroidRuntimeSettings.AndroidRuntimeSettings'
    if not config.has_section(AndroidRuntimeSettings):
        config.add_section(AndroidRuntimeSettings)
    ExtraActivitySettings = '<intent-filter><action android:name="com.google.intent.action.TEST_LOOP"/><category android:name="android.intent.category.DEFAULT"/><data android:mimeType="application/javascript"/></intent-filter>'  
    config[AndroidRuntimeSettings]['ExtraActivitySettings'] = ExtraActivitySettings
    config[AndroidRuntimeSettings]['bSupportAdMob'] = 'False'
    config[AndroidRuntimeSettings]['bPackageDataInsideApk'] = 'True'
    with open(defaultengine,'w') as fw:
        config.write(CustomWriter(fw))


if __name__ == "__main__":
    change_runtime_settings(sys.argv[1])
