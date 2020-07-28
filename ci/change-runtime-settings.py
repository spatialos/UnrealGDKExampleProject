# coding=utf-8

# Purpose:
#   change runtime settings for build android and ios package

import os
import sys
import platform
import common

# parse Default_Engine.ini or other ini files
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
    default_engine = os.path.join(project_home, 'Game', 'Config', 'DefaultEngine.ini')
    ios_runtime_settings = '/Script/IOSRuntimeSettings.IOSRuntimeSettings'
    android_runtime_settings = '/Script/AndroidRuntimeSettings.AndroidRuntimeSettings'
    ini = IniParser()
    ini.open(default_engine)

    # for ios runtime settings
    additional_plist_data = '<key>CFBundleURLTypes</key><array><dict><key>CFBundleURLName</key><string></string><key>CFBundleTypeRole</key><string>Editor</string><key>CFBundleURLSchemes</key><array><string>firebase-game-loop</string></array></dict></array>'
    ini.add(ios_runtime_settings, 'AdditionalPlistData', additional_plist_data)
    ini.add(ios_runtime_settings, 'BundleIdentifier', 'io.improbable.unrealgdkdemo')
    ini.add(ios_runtime_settings, 'MobileProvision', '')
    ini.add(ios_runtime_settings, 'BundleDisplayName', 'UnrealGDK Shooter')
    ini.add(ios_runtime_settings, 'BundleName', 'unrealgdkshooter')
    ini.add(ios_runtime_settings, 'bSupportsITunesFileSharing', 'True')
    ini.add(ios_runtime_settings, 'bGeneratedSYMFile', 'True')
    ini.add(ios_runtime_settings, 'bGeneratedSYMBundle', 'True')
    ini.add(ios_runtime_settings, 'bGenerateCrashReportSymbols', 'True')
    ini.add(ios_runtime_settings, 'bEnableRemoteNotificationsSupport', 'False')
    ini.add(ios_runtime_settings, 'bEnableAdvertisingIdentifier', 'False')
    ini.add(ios_runtime_settings, 'bEnableCloudKitSupport', 'False')

    # for Android extra activity settings
    extra_activity_settings = '<intent-filter><action android:name="com.google.intent.action.TEST_LOOP"/><category android:name="android.intent.category.DEFAULT"/><data android:mimeType="application/javascript"/></intent-filter>'
    ini.add(android_runtime_settings, 'ExtraActivitySettings', extra_activity_settings)
    ini.add(android_runtime_settings, 'bSupportAdMob', 'False')
    ini.add(android_runtime_settings, 'bPackageDataInsideApk', 'True')

    ini.write(default_engine)


if __name__ == "__main__":
    change_runtime_settings(sys.argv[1])
