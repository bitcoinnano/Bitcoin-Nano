Btcnano Nano version 0.14.5 is now available from:

  <https://download.btcnano.org/0.14.5/>

This release includes the following features and fixes:

- Enforce strong replay protection (require SIGHASH_FORKID
  and SCRIPT_VERIFY_STRICTENC compliance)
- Change currency units in user interface from BTN -> BTN
- Add NODE_BTCNANO_CASH service bit (0x20)
- Update BU backed seeder to btncash-seeder.btcnanounlimited.info
- Update ABC logos for About menu, and testnet icon 
- Various refactoring and cleanups

NOTE: Change in default value:
- 'forcednsseed' is enabled by default to increase
  chance of good connection to UAHF network (see D360)
