# coding=utf-8

# Purpose:
#   change runtime settings for build android and ios package


import os
import sys
import common

# parse DefaultEngine.ini or other ini files
class IniParser:
    def __init__(self):
        self.change_groups = {}
        self.keep_groups = {}

    def open(self, filename):
        with open(filename, 'r') as fr:
            group = None
            group_type = 0
            for l in fr.readlines():
                line = l.strip()
                if line.startswith('[') and line.endswith(']'):
                    name = line[1:-1]
                    if 'IOSRuntimeSettings.IOSRuntimeSettings' in line or 'AndroidRuntimeSettings.AndroidRuntimeSettings' in line:                        
                        group = {}
                        self.change_groups[name] = group
                        group_type = 1
                    else:                        
                        group = []
                        self.keep_groups[name] = group
                        group_type = 2
                    continue
                if group_type == 1:
                    if '=' in line and group != None:
                        k, v = line.split('=', 1)
                        group[k] = v
                elif group_type == 2:
                    group.append(line)


    def write(self, filename):
        with open(filename, 'w') as fw:
            for key, group in self.change_groups.items():
                self.write_impl(fw, '[%s]\n' % key)
                if group != None:
                    for k, v in group.items():
                        self.write_impl(fw, '%s=%s\n' % (k, v))
                    self.write_impl(fw, '\n')
                    
            for key, group in self.keep_groups.items():
                self.write_impl(fw, '[%s]\n' % key)
                if group != None:
                    for line in group:
                        self.write_impl(fw, '%s\n' % line)
                    self.write_impl(fw, '\n')

    def write_impl(self, fw, s):
        fw.write(s)

    def add(self, group, key, value):
        if group not in self.change_groups:
            self.change_groups[group] = {}
        self.change_groups[group][key] = value


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
