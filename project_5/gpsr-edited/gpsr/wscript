# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

# def options(opt):
#     pass

# def configure(conf):
#     conf.check_nonfatal(header_name='stdint.h', define_name='HAVE_STDINT_H')

def build(bld):
    module = bld.create_ns3_module('gpsr', ['location-service', 'internet', 'wifi', 'applications', 'mesh', 'point-to-point', 'virtual-net-device'])
    module.source = [
        'model/gpsr-ptable.cc',
        'model/gpsr-rqueue.cc',
        'model/gpsr-packet.cc',
        'model/gpsr.cc',
        'helper/gpsr-helper.cc',
        ]

    headers = bld(features=['ns3header'])
    headers.module = 'gpsr'
    headers.source = [
        'model/gpsr-ptable.h',
        'model/gpsr-rqueue.h',
        'model/gpsr-packet.h',
        'model/gpsr.h',
        'helper/gpsr-helper.h',
        ]

    
    # bld.ns3_python_bindings()

