## Create wallet

`cleos wallet create --to-console`
> PW5KBHZL5QhRc9W1q2kcVaeFkL4Dds4JABJgr6o6Pqg3DgmLqkVYV

## Import default private key for accounts to work

`cleos wallet import --private-key 5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3`

## Open wallet

`cleos wallet unlock --password`

## Smart contract wallet
public key: EOS8PtSASYAm6n5XmhEWtAcqcbDyVeG4HU2ogQNna9t6KV9dyz8ue

## Deploying smart contract

First, create the keys:
`cleos wallet create_key`

Then, use this key to generate a smart contract account:
`cleos create account default <acct_name> <public key from previous step>`

From directory with *.abi and *.wasm files:
`cleos set contract <acct_name> . -p <acct_name>@active`

## Calling smart contract actions from EOSIO

`cleos push action <acct_name> <action_name> '[<arg1>, <arg2>, ...]' -p <acct_name>@active`

# Testnet

## Credentials

nkrivenko351 / PW5KbZsvmjVRiKg4Qceukr7Bys7hrggSm4ok1s5RUyJRgoFNLXYCp

## Importing account into cleos

`cleos wallet create -n <account_name> --to-console && cleos wallet import -n <account_name> --private-key <active private key> && cleos wallet import -n <account_name> --private-key <owner private key>`

## Buying RAM

`cleos -u https://testnet.waxsweden.org system buyram <from_acct> <to_acct> "3.00000000 WAX" `

## Staking WAXP for CPU and RAM

`cleos -u https://testnet.waxsweden.org system delegatebw nkrivenko351 nkrivenko351 <NET staking> <CPU staking>`

## Allowing our account to deploy code to testnet

`cleos -u https://testnet.waxsweden.org set account permission nkrivenko351 active --add-code`

## Deploying our contract to testnet

`cleos -u https://testnet.waxsweden.org set contract nkrivenko351 . nft.wasm nft.abi `

## Atomichub

### Create schema

`wtcleos push action atomicassets createschema '["nkrivenko351", "nkrivenkotc3", "tools", [{"name":"name", "type":"string"}, {"name":"category", "type":"string"}, {"name":"img", "type":"string"}, {"name":"max_usages", "type":"uint32"}, {"name": "cd", "type": "uint32"}, {"name": "mint", "type": "double"}, {"name": "energy_consumption", "type": "uint32"}, {"name": "last_usage", "type": "uint32"}, {"name": "used", "type": "uint32"}]]' -p nkrivenko351`

### Create template

`wtcleos push action atomicassets createtempl '["nkrivenko351", "nkrivenkotc3", "simplefilter", true, true, 100,[{"key":"name", "value":["string", "Simple Filter"]}, {"key":"category", "value":["string", "water"]}, {"key":"max_usages", "value":["uint32", 100]}, {"key":"cd", "value":["uint32", 100]}, {"key":"mint", "value":["double", 0.1]}, {"key":"energy_consumption", "value":["uint32", 10]}]]' -p nkrivenko351`

## Fungible tokens

### Create token

```
cleos -u https://testnet.wax.eosdetroit.io push transaction '{
  "delay_sec": 0,
  "max_cpu_usage_ms": 0,
  "actions": [
    {
      "account": "ggttokenwax1",
      "name": "create",
      "data": {
        "issuer": "ggttokenwax1",
        "maximum_supply": "10000.00 TFOOD"
      },
      "authorization": [
        {
          "actor": "ggttokenwax1",
          "permission": "active"
        }
      ]
    }
  ]                               
}'
```

### Issue token

```
cleos -u https://testnet.wax.eosdetroit.io push transaction '{
  "delay_sec": 0,
  "max_cpu_usage_ms": 0,
  "actions": [
    {
      "account": "ggttokenwax1",
      "name": "issue",
      "data": {
        "to": "nkrivenko351",
        "quantity": "100.00 TMETAL",
        "memo": "Init"
      },
      "authorization": [
        {
          "actor": "nkrivenko351",
          "permission": "active"
        }
      ]
    }
  ]
}'
```

#### ggtgamewat12

PW5KL7DMmk2aUobCxCQ6DcweZ5DqydJW3Wp6HZtWyrVK5G7pW3mRs

#### ggtgamewat23

PW5KMSp1kZauJdnztiwNMiJEjvybt4nJ3MMBkVdxewc1wqmsXou68
