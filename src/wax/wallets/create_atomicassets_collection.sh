#!/bin/bash

set -e

alias wtcleos='cleos -v -u https://testnet.wax.pink.gg'
cleos wallet unlock -n testnet

wtcleos push action atomicassets createcol '["cc32dninenft", "loremipsumdo", true, [], [], 0.05, []]' -p cc32dninenft

wtcleos push action atomicassets addcolauth '["loremipsumdo", "cc32dninenft"]' -p cc32dninenft

wtcleos push action atomicassets createschema '["cc32dninenft", "loremipsumdo", "tempor", 
[{"name":"name", "type":"string"}, {"name":"commodo", "type":"string"}, {"name":"consequat", "type":"uint32"}, {"name":"ullamco", "type":"float"}]]' -p cc32dninenft


wtcleos push action atomicassets createtempl '["cc32dninenft", "loremipsumdo", "tempor", true, true, 100,
[{"key":"commodo", "value":["string", "culpa"]}]]' -p cc32dninenft

# response: atomicassets <= atomicassets::lognewtempl    {"template_id":91299,


wtcleos push action atomicassets mintasset '["cc32dninenft", "loremipsumdo", "tempor", 91299, "cc32dninexxx", 
[{"key":"consequat", "value":["uint32", "777"]}], [{"key":"ullamco", "value":["float32", "-0.0005"]}], []]' -p cc32dninenft

# response: atomicassets <= atomicassets::logmint        {"asset_id":"1099514590272"


# https://wax-test.atomichub.io/explorer/asset/1099514590272
