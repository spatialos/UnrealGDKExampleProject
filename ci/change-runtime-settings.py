# coding=utf-8

# Purpose:
#   change runtime settings for build android and ios package


import os
import sys
import common

# parse DefaultEngine.ini or other ini files
class IniParser:
    def __init__(self):
        self.groups = {}

    def open(self, filename):
        with open(filename, 'r') as fr:
            group = None
            for l in fr.readlines():
                line = l.strip()
                if line.startswith('[') and line.endswith(']'):
                    name = line[1:-1]
                    group = {}
                    self.groups[name] = group
                if '=' in line and group != None:
                    k, v = line.split('=', 1)
                    group[k] = v

    def write(self, filename):
        with open(filename, 'w') as fw:
            for key, group in self.groups.items():
                self.write_impl(fw, '[%s]\n' % key)
                if group != None:
                    for k, v in group.items():
                        self.write_impl(fw, '%s=%s\n' % (k, v))
                    self.write_impl(fw, '\n')

    def write_impl(self, fw, s):
        # print(s)
        fw.write(s)

    def add(self, group, key, value):
        if group not in self.groups:
            self.groups[group] = {}
        self.groups[group][key] = value


# modify runtime settings before cook
def change_runtime_settings(project_home):
    event_name = "change_runtime_settings"
    common.start_event(event_name)
    defaultengine = os.path.join(project_home, 'Game', 'Config', 'DefaultEngine.ini')
    print('defaultengine:%s' % defaultengine)
    IOSRuntimeSettings = '/Script/IOSRuntimeSettings.IOSRuntimeSettings'
    AndroidRuntimeSettings = '/Script/AndroidRuntimeSettings.AndroidRuntimeSettings'
    ini = IniParser()
    ini.open(defaultengine)

    # for ios runtime settings
    AdditionalPlistData = '<key>CFBundleURLTypes</key><array><dict><key>CFBundleURLName</key><string></string><key>CFBundleTypeRole</key><string>Editor</string><key>CFBundleURLSchemes</key><array><string>firebase-game-loop</string></array></dict></array>'
    ini.add(IOSRuntimeSettings, 'AdditionalPlistData', AdditionalPlistData)
    ini.add(IOSRuntimeSettings, 'BundleIdentifier',
            'io.improbable.unrealgdkdemo')
    ini.add(IOSRuntimeSettings, 'MobileProvision', '')
    ini.add(IOSRuntimeSettings, 'BundleDisplayName', 'UnrealGDK Shooter')
    ini.add(IOSRuntimeSettings, 'BundleName', 'unrealgdkshooter')
    ini.add(IOSRuntimeSettings, 'bSupportsITunesFileSharing', 'True')
    ini.add(IOSRuntimeSettings, 'bGeneratedSYMFile', 'True')
    ini.add(IOSRuntimeSettings, 'bGeneratedSYMBundle', 'True')
    ini.add(IOSRuntimeSettings, 'bGenerateCrashReportSymbols', 'True')
    ini.add(IOSRuntimeSettings, 'bEnableRemoteNotificationsSupport', 'False')
    ini.add(IOSRuntimeSettings, 'bEnableAdvertisingIdentifier', 'False')
    ini.add(IOSRuntimeSettings, 'bEnableCloudKitSupport', 'False')

    # for Android extra activity settings
    ExtraActivitySettings = '<intent-filter><action android:name="com.google.intent.action.TEST_LOOP"/><category android:name="android.intent.category.DEFAULT"/><data android:mimeType="application/javascript"/></intent-filter>'
    ini.add(AndroidRuntimeSettings, 'ExtraActivitySettings', ExtraActivitySettings)
    ini.add(AndroidRuntimeSettings, 'bSupportAdMob', 'False')
    ini.add(AndroidRuntimeSettings, 'bPackageDataInsideApk', 'True')

    ini.write(defaultengine)
    common.finish_event(event_name)


if __name__ == "__main__":
    print('begin')
    change_runtime_settings(sys.argv[1])
    print('end')
    exit(0)
