#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
A simple http server for testing/debugging the web-UI

open http://localhost:8080/
add the following query params for TX and/or 900Mhz testing
    isTX
    hasSubGHz
"""

from external.bottle import route, run, response, request, static_file
from external.wheezy.template.engine import Engine
from external.wheezy.template.ext.core import CoreExtension
from external.wheezy.template.loader import FileLoader

net_counter = 0
isTX = False
hasSubGHz = False

config = {
        "config": {
            "mode":"STA",
            "ssid":"network-ssid",
            "product_name": "VRX Testing",
            # "aat": {
            #     "servosmoo": 5,
            #     "servomode": 0,
            #     "azim_center": 0,
            #     "azim_min": 500,
            #     "azim_max": 2500,
            #     "elev_min": 1000,
            #     "elev_max": 2000,
            # },
            # "vbat": {
            #     "offset": 292,
            #     "scale": -2
            # }
        }
    }

def apply_template(mainfile):
    global isTX, hasSubGHz
    engine = Engine(
        loader=FileLoader(["html"]),
        extensions=[CoreExtension("@@")]
    )
    template = engine.get_template(mainfile)
    data = template.render({
            'VERSION': 'testing (xxxxxx)',
            'PLATFORM': 'Unified_ESP8285',
            'isTX': isTX,
            'hasSubGHz': hasSubGHz
        })
    return data

@route('/')
def index():
    global net_counter, isTX, hasSubGHz
    net_counter = 0
    isTX = 'isTX' in request.query
    hasSubGHz = 'hasSubGHz' in request.query
    response.content_type = 'text/html; charset=latin9'
    return apply_template('vrx_index.html')

@route('/logo.svg')
def logo():
    response.content_type = 'image/svg+xml; charset=latin9'
    return apply_template('logo.svg')

@route('/elrs.css')
def elrs():
    response.content_type = 'text/css; charset=latin9'
    return apply_template('elrs.css')

@route('/scan.js')
def scan():
    response.content_type = 'text/javascript; charset=latin9'
    return apply_template('scan.js')

@route('/mui.css')
def mui_css():
    response.content_type = 'text/css; charset=latin9'
    return apply_template('mui.css')

@route('/mui.js')
def mui():
    response.content_type = 'text/javascript; charset=latin9'
    return apply_template('mui.js')

@route('/p5.js')
def p5():
    response.content_type = 'text/javascript; charset=latin9'
    return static_file('p5.js', root='html')

@route('/airplane.obj')
def airplane_obj():
    response.content_type = 'text/plain; charset=latin9'
    return static_file('airplane.obj', root='html')

@route('/texture.gif')
def texture_gif():
    response.content_type = 'image/gif; charset=latin9'
    return static_file('texture.gif', root='html')

@route('/config')
def options():
    response.content_type = 'application/json; charset=latin9'
    return config

@route('/config', method='POST')
def update_config():
    if 'button-actions' in request.json:
        config['config']['button-actions'] = request.json['button-actions']
    if 'pwm' in request.json:
        i=0
        for x in request.json['pwm']:
            print(x)
            config['config']['pwm'][i]['config'] = x
            i = i + 1
    if 'protocol' in request.json:
        config['config']['serial-protocol'] = request.json['protocol']
    if 'modelid' in request.json:
        config['config']['modelid'] = request.json['modelid']
    if 'forcetlm' in request.json:
        config['config']['force-tlm'] = request.json['forcetlm']
    return "Config Updated"

@route('/options.json', method='POST')
def update_options():
    config['options'] = request.json
    return "Options Updated"

@route('/import', method='POST')
def import_config():
    json = request.json
    print(json)
    return "Config Updated"

@route('/sethome', method='POST')
def options():
    response.content_type = 'application/json; charset=latin9'
    return "Connecting to network '" + request.forms.get('network') + "', connect to http://elrs_tx.local from a browser on that network"

@route('/networks.json')
def mode():
    global net_counter
    net_counter = net_counter + 1
    if (net_counter > 3):
        return '["Test Network 1", "Test Network 2", "Test Network 3", "Test Network 4", "Test Network 5"]'
    response.status = 204
    return '[]'

if __name__ == '__main__':
    run(host='localhost', port=8080)
