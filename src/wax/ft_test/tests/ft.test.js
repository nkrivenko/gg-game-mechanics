const { loadConfig, Blockchain } = require("@klevoya/hydra");

const config = loadConfig("hydra.yml");

const assetToAmount = (asset, tokenTicker) => {
  return parseFloat(asset.substring(0, asset.lastIndexOf(tokenTicker)));
}

describe("ft", () => {
  let blockchain = new Blockchain(config);
  let tester = blockchain.createAccount(`ggttokenwat1`);

  const user1 = blockchain.createAccount('user1');
  const user2 = blockchain.createAccount('user2');

  beforeAll(async () => {
    tester.setContract(blockchain.contractTemplates[`ft`]);
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
  });

  beforeEach(async () => {
    tester.resetTables();
  });

  describe('addminter', () => {
    it("can send the addminter action", async () => {
      expect.assertions(1);

      await tester.contract.addminter({
        new_minter: user1.accountName
      });

      expect(tester.getTableRowsScoped('minters')[tester.accountName]).toEqual([
        {
          minter: user1.accountName
        }
      ]);
    });

    it("cannot send the addminter action if new_minter already a minter", async () => {
      expect.assertions(2);

      await tester.contract.addminter({
        new_minter: user1.accountName
      });

      expect(tester.getTableRowsScoped('minters')[tester.accountName]).toEqual([
        {
          minter: user1.accountName
        }
      ]);

      await expect(tester.contract.addminter({
        new_minter: user1.accountName
      })).rejects.toThrow("Already a minter");
    });

    it("can send the addminter action only from _self", async () => {
      expect.assertions(1);

      await expect(tester.contract.addminter({
        new_minter: user1.accountName
      }, [{ actor: user1.accountName, permission: 'active' }])).rejects.toThrow("missing authority of ggttokenwat1");
    });
  });

  describe('remminter', () => {
    it("can send the remminter action", async () => {
      expect.assertions(2);

      await tester.contract.addminter({
        new_minter: user1.accountName
      });

      expect(tester.getTableRowsScoped('minters')[tester.accountName]).toEqual([
        {
          minter: user1.accountName
        }
      ]);

      await tester.contract.remminter({
        old_minter: user1.accountName
      });

      expect(tester.getTableRowsScoped('minters')[tester.accountName]).toBeFalsy();
    });

    it('can send the remminter action only from _self', async () => {
      expect.assertions(2);

      await tester.contract.addminter({
        new_minter: user1.accountName
      });

      expect(tester.getTableRowsScoped('minters')[tester.accountName]).toEqual([
        {
          minter: user1.accountName
        }
      ]);

      await expect(tester.contract.remminter({
        old_minter: user1.accountName
      }, [{ actor: user1.accountName, permission: 'active' }])).rejects.toThrow("missing authority of ggttokenwat1");
    });

    it('cannot send the remminter action if account is not a minter', async () => {
      expect.assertions(1);

      await expect(tester.contract.remminter({
        old_minter: user1.accountName
      })).rejects.toThrow("Not a minter");
    });
  });

  describe('transfers', () => {
    beforeEach(async () => {
      await tester.loadFixtures();
    });

    it('can send transfers action', async () => {
      expect.assertions(4);

      const tokensToTransfer = 1;

      const transfersInfo = {
        from: user1.accountName,
        to: user2.accountName,
        quantities: [`${tokensToTransfer}.0000 GM`, `${tokensToTransfer}.0000 GW`],
        memo: ""
      };

      const oldSenderAmounts = tester.getTableRowsScoped('accounts')[user1.accountName];
      const oldRecipientAmounts = tester.getTableRowsScoped('accounts')[user1.accountName];

      await tester.contract.transfers(transfersInfo, [{ actor: user1.accountName, permission: 'active' }]);

      const newSenderAmounts = tester.getTableRowsScoped('accounts')[user1.accountName];
      const newRecipientAmounts = tester.getTableRowsScoped('accounts')[user1.accountName];

      expect(assetToAmount(newSenderAmounts[0]["balance"])).toEqual(assetToAmount(oldSenderAmounts[0]["balance"]) - tokensToTransfer);
      expect(assetToAmount(newSenderAmounts[1]["balance"])).toEqual(assetToAmount(oldSenderAmounts[1]["balance"]) - tokensToTransfer);

      expect(assetToAmount(newRecipientAmounts[0]["balance"])).toEqual(assetToAmount(oldRecipientAmounts[0]["balance"]) + tokensToTransfer);
      expect(assetToAmount(newRecipientAmounts[1]["balance"])).toEqual(assetToAmount(oldRecipientAmounts[1]["balance"]) + tokensToTransfer);
    });

    it('cannot send transfers action if from == to', async () => {
      expect.assertions(1);

      const transfersInfo = {
        from: user1.accountName,
        to: user1.accountName,
        quantities: ["1.0000 GM", "1.0000 GW"],
        memo: ""
      };

      await expect(tester.contract.transfers(transfersInfo, [{ actor: user1.accountName, permission: 'active' }]))
        .rejects.toThrow("cannot transfer to self");
    });

    it('cannot send transfers action with no quantities', async () => {
      expect.assertions(1);

      const transfersInfo = {
        from: user1.accountName,
        to: user2.accountName,
        quantities: [],
        memo: ""
      };

      await expect(tester.contract.transfers(transfersInfo, [{ actor: user1.accountName, permission: 'active' }]))
        .rejects.toThrow("quantities must not be empty");
    });

    it('cannot send transfers action if destination does not exist', async () => {
      expect.assertions(1);

      const transfersInfo = {
        from: user1.accountName,
        to: 'user5',
        quantities: ["1.0000 GM", "1.0000 GW"],
        memo: ""
      }

      await expect(tester.contract.transfers(transfersInfo, [{ actor: user1.accountName, permission: 'active' }]))
        .rejects.toThrow("to account does not exist");
    });

    it('cannot send transfers action if any quantity is negative', async () => {
      expect.assertions(1);

      const transfersInfo = {
        from: user1.accountName,
        to: user2.accountName,
        quantities: ["1.0000 GM", "-1.0000 GW"],
        memo: ""
      }

      await expect(tester.contract.transfers(transfersInfo, [{ actor: user1.accountName, permission: 'active' }]))
        .rejects.toThrow("must transfer positive quantity");
    });

    it('cannot send transfers action if precision mismatch', async () => {
      expect.assertions(1);

      const transfersInfo = {
        from: user1.accountName,
        to: user2.accountName,
        quantities: ["1.0000 GM", "1.00000 GW"],
        memo: ""
      }

      await expect(tester.contract.transfers(transfersInfo, [{ actor: user1.accountName, permission: 'active' }]))
        .rejects.toThrow("symbol precision mismatch");
    });

    it('cannot send transfers action if from unauthorized', async () => {
      expect.assertions(1);

      const transfersInfo = {
        from: user1.accountName,
        to: user2.accountName,
        quantities: ["1.0000 GM", "1.0000 GW"],
        memo: ""
      }

      await expect(tester.contract.transfers(transfersInfo, [{ actor: user2.accountName, permission: 'active' }]))
        .rejects.toThrow(`missing authority of ${user1.accountName}`);
    });
  });

  describe('burntransfer', () => {
    it('can send burntransfer action', async () => {
      await tester.loadFixtures();

      expect.assertions(5);

      const amountToSplit = 10;
      const quarter = amountToSplit / 4;

      const burntransferInfo = {
        from: user1.accountName,
        to: user2.accountName,
        quantities: [`10.0000 GM`],
        transfer_percent: 25,
        memo: ""
      };

      const oldTesterBalances = tester.getTableRowsScoped('accounts')[user1.accountName];
      const oldUser2Balances = tester.getTableRowsScoped('accounts')[user2.accountName];
      const oldStats = tester.getTableRowsScoped('stat')['GM'];

      await tester.contract.burntransfer(burntransferInfo, [{actor: user1.accountName, permission: 'active'}]);

      const newTesterBalances = tester.getTableRowsScoped('accounts')[user1.accountName];
      const newUser2Balances = tester.getTableRowsScoped('accounts')[user2.accountName];

      expect(assetToAmount(oldTesterBalances[0]['balance'])).toEqual(assetToAmount(newTesterBalances[0]['balance']) - amountToSplit);
      expect(assetToAmount(oldTesterBalances[1]['balance'])).toEqual(assetToAmount(newTesterBalances[1]['balance']) - amountToSplit);

      expect(assetToAmount(oldUser2Balances[0]['balance'])).toEqual(assetToAmount(newUser2Balances[0]['balance']) + quarter);
      expect(assetToAmount(oldUser2Balances[1]['balance'])).toEqual(assetToAmount(newUser2Balances[0]['balance']) + quarter);

      const newStats = tester.getTableRowsScoped('stat')['GM'];
      expect(newStats[0].supply).toEqual(`${assetToAmount(oldStats[0].supply, 'GM') - 3 * quarter}000 GM`);
    });

    [
      { percent: 0, caption: 'zero' },
      { percent: -1, caption: 'negative' },
      { percent: 101, caption: 'more than 100' }
    ].forEach(pct => {
      it(`cannot send burntransfer action if percent is ${pct.caption}`, async () => {
        expect.assertions(1);

        const burntransferInfo = {
          from: user1.accountName,
          to: user2.accountName,
          quantities: ["100.0000 GM", "100.0000 GW"],
          transfer_percent: pct.percent,
          memo: ""
        };

        await expect(tester.contract.burntransfer(burntransferInfo, [ {actor: user1.accountName, permission: 'active'} ]))
          .rejects.toThrow('Transfer percent must be positive number less than or equal to 100');
      });
    });
  });

  describe('issue', () => {
    it('can mint to account', async () => {
      expect.assertions(2);

      await tester.loadFixtures();

      const tokenSymbol = "GM";
      const tokenAmount = 100;

      const issueInfo = {
        to: user1.accountName,
        quantity: `${tokenAmount}.0000 ${tokenSymbol}`,
        memo: ""
      };

      const oldStats = tester.getTableRowsScoped('stat')[tokenSymbol][0].supply;
      const user1OldBalance = tester.getTableRowsScoped('accounts')[user1.accountName][0].balance;

      await tester.contract.issue(issueInfo);

      const newStats = tester.getTableRowsScoped('stat')[tokenSymbol][0].supply;
      const user1NewBalance = tester.getTableRowsScoped('accounts')[user1.accountName][0].balance;

      expect(assetToAmount(oldStats, tokenSymbol)).toEqual(assetToAmount(newStats, tokenSymbol) - tokenAmount)
      expect(assetToAmount(user1NewBalance, tokenSymbol)).toEqual(assetToAmount(user1OldBalance, tokenSymbol) + tokenAmount);
    });

    it('can mint only if called by minter', async () => {
      expect.assertions(1);

      const issueInfo = {
        to: user1.accountName,
        quantity: "100.0000 GM",
        memo: ""
      };

      await expect(tester.contract.issue(issueInfo, [{ actor: user1.accountName, permission: 'active' }]))
        .rejects.toThrow("Unauthorized");
    });
  })
});
