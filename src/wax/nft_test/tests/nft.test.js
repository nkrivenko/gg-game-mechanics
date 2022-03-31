const { loadConfig, Blockchain } = require("@klevoya/hydra");
const { deserialize, ObjectSchema } = require("atomicassets");
const fs = require("fs");

const config = loadConfig("hydra.yml");

const OBJECT_SCHEMA = ObjectSchema(JSON.parse(fs.readFileSync("tests/fixtures/atomicassets/schemas.json", "utf8"))["nkrivenkotc3"][0]["format"]);

const assetToAmount = (asset, tokenTicker) => {
  return parseFloat(asset.substring(0, asset.lastIndexOf(tokenTicker)));
}

describe("nft", () => {
  let blockchain = new Blockchain(config);

  let tester = blockchain.createAccount(`nft`);
  let token = blockchain.createAccount(`ggttokenwat1`);
  let atomicassets = blockchain.createAccount(`atomicassets`);

  let user1 = blockchain.createAccount('user1');
  let user2 = blockchain.createAccount('user2');
  let user3 = blockchain.createAccount('user3');

  let nkrivenko351 = blockchain.createAccount('nkrivenko351');

  const initContract = (contractAccount, templateName) => {
    tester.setContract(blockchain.contractTemplates[`nft`]);
    tester.updateAuth(`active`, `owner`, {
      accounts: [
        {
          permission: {
            actor: tester.accountName,
            permission: `eosio.code`
          },
          weight: 1
        }
      ]
    });
  }

  beforeAll(async () => {
    initContract(tester, 'nft');
    initContract(token, 'ft');
    initContract(atomicassets, 'atomicassets');
  });

  beforeEach(async () => {
    tester.resetTables();
    token.resetTables();
    atomicassets.resetTables();
  });

  it("can send consumefood", async () => {
    expect.assertions(2);
    await tester.loadFixtures();

    const accountName = user1.accountName;
    const oldAccount = tester.getTableRowsScoped('accounts')['nft'].find(t => t.owner === accountName);

    await tester.contract.consumefood({
      account: accountName,
      food_to_consume: "1.0000 GF"
    });

    const newAccount = tester.getTableRowsScoped('accounts')['nft'].find(t => t.owner === accountName);

    expect(newAccount.energy).toEqual(oldAccount.energy + 5);
    expect(newAccount.quantities[0]).toEqual("122.4500 GF");
  });

  [
    { amount: "200.0000 GF", caption: "insufficient", error_message: "Not enough food" },
    { amount: "-1.0000 GF", caption: "negative", error_message: "Amount of food must be positive" },
    { amount: "0.0000 GF", caption: "zero", error_message: "Amount of food must be positive" }
  ].forEach(amt => it(`will revert if consumefood sent with ${amt.caption} funds`, async () => {
    expect.assertions(1);
    await tester.loadFixtures();

    await expect(tester.contract.consumefood({
      account: user1.accountName,
      food_to_consume: amt.amount
    })).rejects.toThrow(amt.error_message);
  }));

  it("will reject if food consumed by unknown account", async () => {
    expect.assertions(1);

    await expect(tester.contract.consumefood({
      account: user2.accountName,
      food_to_consume: "1.0000 GF"
    })).rejects.toThrow("No account");
  });

  it("will reject consumefood if exceeds the energy limit", async () => {
    expect.assertions(1);
    await tester.loadFixtures();

    await expect(tester.contract.consumefood({
      account: user3.accountName,
      food_to_consume: "20001.0000 GF"
    })).rejects.toThrow("Energy limit exceeded");
  });

  it("can insert nft configs through addasset", async () => {
    expect.assertions(1);

    const newAssetInfo = {
      asset_code: "user1",
      collection_name: "user1",
      schema_name: "user1",
      template_id: 123,
      craft_energy_consumption: 123,
      repair_energy_consumption: 123,
      craft_resources: ["1.2345 GF"],
      repair_resources: ["1.2345 GF"],
      description: "English description",
      comment: "English comment"
    };

    await tester.contract.addasset(newAssetInfo);

    expect(tester.getTableRowsScoped(`nfts`)[`nft`]).toEqual([newAssetInfo]);
  });

  it('can update nft configs through addasset', async () => {
    expect.assertions(3);
    await tester.loadFixtures();

    const assets = tester.getTableRowsScoped('nfts')['nft'];
    const asset = assets[0];

    const newCraftEnergyConsumption = parseInt(asset.craft_energy_consumption) + 2;
    const newRepairEnergyConsumption = parseInt(asset.repair_energy_consumption) + 2;

    const updateAssetInfo = {
      asset_code: asset.asset_code,
      collection_name: asset.collection_name,
      schema_name: asset.schema_name,
      template_id: asset.template_id,
      craft_energy_consumption: newCraftEnergyConsumption,
      repair_energy_consumption: newRepairEnergyConsumption,
      craft_resources: asset.craft_resources,
      repair_resources: asset.repair_resources,
      description: asset.description,
      comment: asset.comment
    };

    await tester.contract.addasset(updateAssetInfo);

    const updatedAsset = tester.getTableRowsScoped(`nfts`)[`nft`];

    expect(updatedAsset.length).toBe(assets.length);
    expect(updatedAsset[0].craft_energy_consumption).toEqual(newCraftEnergyConsumption);
    expect(updatedAsset[0].repair_energy_consumption).toEqual(newRepairEnergyConsumption);
  });

  it('can remove nft configs through remasset', async () => {
    expect.assertions(3);
    await tester.loadFixtures();

    const oldAssets = tester.getTableRowsScoped('nfts')['nft'];

    expect(oldAssets.length).toBeGreaterThan(0);

    const remAssetData = {
      asset_code: oldAssets[0].asset_code
    }

    await tester.contract.remasset(remAssetData);

    const newAssets = tester.getTableRowsScoped('nfts')['nft'];

    expect(newAssets.length).toBe(oldAssets.length - 1);
    expect(newAssets).not.toContain(oldAssets[0]);
  });

  it("can craft NFT", async () => {
    await tester.loadFixtures();
    await atomicassets.loadFixtures();

    expect.assertions(4);
    const oldAssets = atomicassets.getTableRowsScoped('assets')[user1.accountName];
    const oldAccount = tester.getTableRowsScoped('accounts')[tester.accountName].find(t => t.owner === user1.accountName);

    const craftInfo = {
      asset_code: "simplefilter",
      asset_owner: user1.accountName
    };

    await tester.contract.craft(craftInfo);

    const newAssets = atomicassets.getTableRowsScoped('assets')[user1.accountName];

    expect(newAssets.length).toEqual((oldAssets.length || 0) + 1);
    expect(newAssets[0].asset_id).toBe("1099511627776");

    const newAccount = tester.getTableRowsScoped('accounts')[tester.accountName].find(t => t.owner === user1.accountName);
    expect(newAccount.energy).toBe(oldAccount.energy - 3);

    expect(newAccount.quantities).toStrictEqual(["111.4500 GF", "113.4500 GM", "123.4500 GW"]);
  });

  it("cannot craft NFT if insufficient balance", async () => {
    await tester.loadFixtures();

    expect.assertions(1);

    const craftInfo = {
      asset_code: "reverseosmos",
      asset_owner: user1.accountName
    };

    await expect(tester.contract.craft(craftInfo)).rejects.toThrow("Insufficient funds");
  });

  it("cannot craft NFT if insufficient energy", async () => {
    await tester.loadFixtures();

    expect.assertions(1);

    const craftInfo = {
      asset_code: "boiler",
      asset_owner: user1.accountName
    };

    await expect(tester.contract.craft(craftInfo)).rejects.toThrow("Insufficient energy");
  });

  it("can repair NFT", async () => {
    expect.assertions(4);

    await atomicassets.loadFixtures();
    await tester.loadFixtures();

    const asset_id = 1099529415673;

    const repairInfo = {
      asset_owner: user3.accountName,
      asset_id
    };

    const oldAccount = tester.getTableRowsScoped('accounts')[tester.accountName].find(t => t.owner === user3.accountName);

    await tester.contract.repair(repairInfo);

    const repairedAsset = atomicassets.getTableRowsScoped('assets')[tester.accountName].find(t => parseInt(t.asset_id) === asset_id);

    expect(repairedAsset).toBeTruthy();
    expect(repairedAsset.mutable_serialized_data).toStrictEqual([13, 0, 14, 0]);

    const newAccount = tester.getTableRowsScoped('accounts')[tester.accountName].find(t => t.owner === user3.accountName);
    expect(newAccount.energy).toBe(oldAccount.energy - 1);

    expect(newAccount.quantities).toStrictEqual(["49997.0000 GF", "121.4500 GM", "123.4500 GW"]);
  });

  it("cannot repair NFT if insufficient balance", async () => {
    expect.assertions(1);

    await atomicassets.loadFixtures();
    await tester.loadFixtures();

    const repairInfo = {
      asset_owner: user1.accountName,
      asset_id: 1099529414676
    };

    await expect(tester.contract.repair(repairInfo)).rejects.toThrow("Insufficient funds");
  });

  it("cannot repair NFT if insufficient energy", async () => {
    expect.assertions(1);

    await atomicassets.loadFixtures();
    await tester.loadFixtures();

    const repairInfo = {
      asset_owner: user1.accountName,
      asset_id: 1099529415630
    };

    await expect(tester.contract.repair(repairInfo)).rejects.toThrow("Insufficient energy");
  });

  it("cannot repair NFT if NFT not broken", async () => {
    await tester.loadFixtures();
    await atomicassets.loadFixtures();

    expect.assertions(1);

    const repairInfo = {
      asset_owner: user1.accountName,
      asset_id: 1099528372160
    };

    await expect(tester.contract.repair(repairInfo)).rejects.toThrow("Not broken");
  });

  it("cannot repair NFT if NFT on cooldown", async () => {
    expect.assertions(1);
    await atomicassets.loadFixtures();
    await tester.loadFixtures();

    const repairInfo = {
      asset_owner: user1.accountName,
      asset_id: 1099529415649
    };

    await expect(tester.contract.repair(repairInfo)).rejects.toThrow("On cooldown");
  });

  it("can mine tokens using the asset", async () => {
    await token.loadFixtures();
    await tester.loadFixtures();
    await atomicassets.loadFixtures();

    expect.assertions(4);

    const asset_id = 1099528372160;

    const mineInfo = {
      asset_owner: user1.accountName,
      asset_id,
      memo: ""
    };

    const oldAssetStats = atomicassets.getTableRowsScoped('assets')[tester.accountName].find(t => parseInt(t.asset_id) === asset_id);
    const oldBalance = token.getTableRowsScoped('accounts')[user1.accountName].find(t => t.balance.endsWith("GW"));
    const oldAccountStats = tester.getTableRowsScoped('accounts')[tester.accountName].find(t => t.owner === user1.accountName);

    await tester.contract.mine(mineInfo);

    const newAccountStats = tester.getTableRowsScoped('accounts')[tester.accountName].find(t => t.owner === user1.accountName);
    const newBalance = token.getTableRowsScoped('accounts')[user1.accountName].find(t => t.balance.endsWith("GW"));
    const newAssetStats = atomicassets.getTableRowsScoped('assets')[tester.accountName].find(t => parseInt(t.asset_id) === asset_id);

    const balance = parseFloat(newBalance.balance.substring(0, newBalance.balance.indexOf(' ')));
    const oldBalanceAsset = parseFloat(oldBalance.balance.substring(0, oldBalance.balance.indexOf(' ')));

    expect(balance).toBe(oldBalanceAsset + 0.1);

    expect(newAccountStats.energy).toBe(oldAccountStats.energy - 1);

    const oldAssetsMutableData = deserialize(oldAssetStats.mutable_serialized_data, OBJECT_SCHEMA);
    const newAssetsMutableData = deserialize(newAssetStats.mutable_serialized_data, OBJECT_SCHEMA);

    expect(newAssetsMutableData["used"]).toBe(oldAssetsMutableData["used"] + 1);
    expect(newAssetsMutableData["last_usage"]).toBeGreaterThan(oldAssetsMutableData["last_usage"]);
  });

  it("cannot mine tokens using the broken asset", async () => {
    expect.assertions(1);

    await atomicassets.loadFixtures();
    await tester.loadFixtures();

    const mineInfo = {
      asset_owner: user1.accountName,
      asset_id: 1099529415673,
      memo: ""
    };

    await expect(tester.contract.mine(mineInfo)).rejects.toThrow("Broken");
  });

  it("cannot mine tokens using the asset on cooldown", async () => {
    expect.assertions(1);

    await atomicassets.loadFixtures();
    await tester.loadFixtures();

    const mineInfo = {
      asset_owner: user1.accountName,
      asset_id: 1099529415674,
      memo: ""
    };

    await expect(tester.contract.mine(mineInfo)).rejects.toThrow("On cooldown");
  });

  it("cannot mine tokens if insufficient energy", async () => {
    expect.assertions(1);
    await atomicassets.loadFixtures();
    await tester.loadFixtures();

    const mineInfo = {
      asset_owner: user3.accountName,
      asset_id: 1099529415915,
      memo: ''
    };

    await expect(tester.contract.mine(mineInfo)).rejects.toThrow("Insufficient energy");
  });

  it("can receive tokens", async () => {
    await tester.loadFixtures();
    await token.loadFixtures();

    expect.assertions(10);

    const amount = 10;

    const transfersInfo = {
      from: user1.accountName,
      to: tester.accountName,
      quantities: [`${amount}.0000 GF`, `${amount}.0000 GM`, `${amount}.0000 GW`],
      memo: ""
    };

    const oldTokenSupply = token.getTableRowsScoped('stat');
    const oldCommissionReceiverBalance = token.getTableRowsScoped('accounts')[nkrivenko351.accountName];

    await token.contract.transfers(transfersInfo, [{ actor: user1.accountName, permission: 'active' }]);
    const newAccount = tester.getTableRowsScoped('accounts')['nft'].find(t => t.owner === user1.accountName);

    expect(newAccount.quantities.length).toBe(3);
    expect(newAccount.quantities[0]).toEqual("133.4500 GF");
    expect(newAccount.quantities[1]).toEqual("133.4500 GM");
    expect(newAccount.quantities[2]).toEqual("133.4500 GW");

    const newTokenSupply = token.getTableRowsScoped('stat');
    const newCommissionReceiverBalance = token.getTableRowsScoped('accounts')[nkrivenko351.accountName];

    expect(assetToAmount(newTokenSupply['GF'][0].supply, "GF"))
      .toEqual(assetToAmount(oldTokenSupply['GF'][0].supply, "GF") - amount * 0.85);
    expect(assetToAmount(newTokenSupply['GM'][0].supply, "GM"))
      .toEqual(assetToAmount(oldTokenSupply['GM'][0].supply, "GM") - amount * 0.85);
    expect(assetToAmount(newTokenSupply['GW'][0].supply, "GW"))
      .toEqual(assetToAmount(oldTokenSupply['GW'][0].supply, "GW") - amount * 0.85);

    console.log(`${JSON.stringify(token.getTableRowsScoped('accounts'))}`)

    expect(assetToAmount(newCommissionReceiverBalance[0].balance, "GF"))
      .toEqual(assetToAmount(oldCommissionReceiverBalance[0].balance, "GF") + amount * 0.15);
    expect(assetToAmount(newCommissionReceiverBalance[1].balance, "GM"))
      .toEqual(assetToAmount(oldCommissionReceiverBalance[1].balance, "GM") + amount * 0.15);
    expect(assetToAmount(newCommissionReceiverBalance[2].balance, "GW"))
      .toEqual(assetToAmount(oldCommissionReceiverBalance[2].balance, "GW") + amount * 0.15);
  });

  it("can receive multiple tokens from new user", async () => {
    await tester.loadFixtures();
    await token.loadFixtures();

    expect.assertions(4);

    const transfersInfo = {
      from: user2.accountName,
      to: tester.accountName,
      quantities: ["10.0000 GF", "10.0000 GM", "10.0000 GW", "10.0000 GW", "11.0000 GF"],
      memo: ""
    };

    await token.contract.transfers(transfersInfo, [{ actor: user2.accountName, permission: 'active' }]);

    const newAccount = tester.getTableRowsScoped('accounts')['nft'].find(t => t.owner === user2.accountName);

    expect(newAccount.quantities.length).toBe(3);
    expect(newAccount.quantities[0]).toEqual("21.0000 GF");
    expect(newAccount.quantities[1]).toEqual("10.0000 GM");
    expect(newAccount.quantities[2]).toEqual("20.0000 GW");
  });

  it("can receive multiple tokens from existing user", async () => {
    await tester.loadFixtures();
    await token.loadFixtures();

    expect.assertions(4);

    const transfersInfo = {
      from: user1.accountName,
      to: tester.accountName,
      quantities: ["10.0000 GF", "10.0000 GM", "10.0000 GW", "10.0000 GW", "11.0000 GF"],
      memo: ""
    };

    await token.contract.transfers(transfersInfo, [{ actor: user1.accountName, permission: 'active' }]);

    const newAccount = tester.getTableRowsScoped('accounts')['nft'].find(t => t.owner === user1.accountName);

    expect(newAccount.quantities.length).toBe(3);
    expect(newAccount.quantities[0]).toEqual("144.4500 GF");
    expect(newAccount.quantities[1]).toEqual("133.4500 GM");
    expect(newAccount.quantities[2]).toEqual("143.4500 GW");
  });

  it('can stake nfts from atomicassets', async () => {
    await tester.loadFixtures();
    await atomicassets.loadFixtures();

    expect.assertions(3);

    const asset_id = 1099528379478;

    const transferInfo = {
      from: user1.accountName,
      to: tester.accountName,
      asset_ids: [asset_id],
      memo: ''
    }
    const oldAmount = tester.getTableRowsScoped('amounts')[tester.accountName].find(t => t.asset_owner === user1.accountName);

    await atomicassets.contract.transfer(transferInfo, [{ actor: user1.accountName, permission: 'active' }]);

    const newAmount = tester.getTableRowsScoped('amounts')[tester.accountName].find(t => t.asset_owner === user1.accountName);
    const asset = tester.getTableRowsScoped('assets')[tester.accountName].find(t => t.asset_owner === user1.accountName && t.asset_id === asset_id.toString());
    const atomicassetsAsset = atomicassets.getTableRowsScoped('assets')[tester.accountName].find(t => t.asset_id === asset_id.toString());

    expect(newAmount).toEqual({ id: "1", asset_owner: user1.accountName, category: 'water', nft_amount: (1 + parseInt(oldAmount.nft_amount)).toString() });
    expect(asset).toEqual({ asset_owner: user1.accountName, category: 'water', asset_id: asset_id.toString() });
    expect(atomicassetsAsset).toBeTruthy();
  });

  it('cannot stake nfts from atomicassets if there are too many for user and category', async () => {
    await tester.loadFixtures();
    await atomicassets.loadFixtures();

    expect.assertions(1);

    const asset_id = 1099528379479;

    const transferInfo = {
      from: user3.accountName,
      to: tester.accountName,
      asset_ids: [asset_id],
      memo: ''
    };

    await expect(atomicassets.contract.transfer(transferInfo, [{ actor: user3.accountName, permission: 'active' }]))
      .rejects.toThrow("Too many NFTs of one category on one account");
  });

  it('can unstake nfts back to owner', async () => {
    await tester.loadFixtures();
    await atomicassets.loadFixtures();

    const asset_id = 1099528372160;
    const oldAsset = tester.getTableRowsScoped('assets')[tester.accountName];
    const unstakeInfo = {
      asset_owner: user1.accountName,
      asset_id
    };

    const oldAmount = tester.getTableRowsScoped('amounts')[tester.accountName].find(t => t.asset_owner === user1.accountName);

    await tester.contract.unstakenft(unstakeInfo);

    const newAmount = tester.getTableRowsScoped('amounts')[tester.accountName].find(t => t.asset_owner === user1.accountName);
    const asset = tester.getTableRowsScoped('assets')[tester.accountName];
    const atomicassetsAsset = atomicassets.getTableRowsScoped('assets')[user1.accountName].find(t => t.asset_id === asset_id.toString());

    expect(newAmount).toEqual({ id: "1", asset_owner: user1.accountName, category: 'water', nft_amount: (parseInt(oldAmount.nft_amount) - 1).toString() });
    expect(asset.length).toBe(oldAsset.length - 1);
    expect(atomicassetsAsset).toBeTruthy();
  });

  it('cannot unstake if wrong owner', async () => {
    await tester.loadFixtures();
    expect.assertions(1);

    const asset_id = 1099528372160;

    const unstakeInfo = {
      asset_owner: user2.accountName,
      asset_id
    };

    await expect(tester.contract.unstakenft(unstakeInfo)).rejects.toThrow("Asset owner does not have NFT");
  });

  it('cannot unstake if owner does not exist', async () => {
    expect.assertions(1);

    const asset_id = 1099528372161;

    const unstakeInfo = {
      asset_owner: user1.accountName,
      asset_id
    };

    await expect(tester.contract.unstakenft(unstakeInfo)).rejects.toThrow("Asset owner does not exist");
  });
});
