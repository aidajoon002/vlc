# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

# def options(opt):
#     pass

# def configure(conf):
#     conf.check_nonfatal(header_name='stdint.h', define_name='HAVE_STDINT_H')

def build(bld):
    module = bld.create_ns3_module('new-module', ['internet', 'mobility', 'aodv'])
    module.source = [
        'model/yans-vlc-channel.cc',
        'model/vlc-channel.cc',
        'model/yans-vlc-phy.cc',
        'model/vlc-phy.cc',
        'model/vlc-phy-state-helper.cc',
        'model/ssid.cc',
        'model/sta-vlc-mac.cc',
        'model/ap-vlc-mac.cc',
        'model/vlc-net-device.cc',
        'model/vlc-mac.cc',
        ]

    module_test = bld.create_ns3_module_test_library('new-module')
    module_test.source = [
        'test/VLC-test-suite.cc',
        ]

    headers = bld(features='ns3header')
    headers.module = 'new-module'
    headers.source = [
        'model/yans-vlc-channel.h',
        'model/vlc-channel.h',
        'model/yans-vlc-phy.h',
        'model/vlc-phy.h',
        'model/vlc-phy-state-helper.h',
        'model/ssid.h',
        'model/sta-vlc-mac.h',
        'model/ap-vlc-mac.h',
        'model/vlc-net-device.h',
        'model/vlc-mac.h',
        ]

    if bld.env.ENABLE_EXAMPLES:
        bld.recurse('examples')

    # bld.ns3_python_bindings()

