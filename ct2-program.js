// MIT project patch code, rebuilt from src/; no firmware is read by the builder.
// Zero replay slots are populated from the user input only.
const hex =
  "ffffffffffffffff70b592f874401d46a4f16503242b01d80f2d06d9174b5c6a" +
  "2b46a446bde87040604700f067f9144a652c02eb4502938805d1002b0cbf0020" +
  "6ff0004070bd662c08d1003b18bf0123002b14bf00206ff00040f3e7074a2244" +
  "12f8672c02f00f01b3fbf1f001fb1033b3eb121f0cbf01230023e9e7fcae1b60" +
  "005020200f292de9f84394bf682500251c4f0c4698bf4d431b4b97f8c8600544" +
  "f6b2d5f8bc909b6a98470f2c804626d800f024f9b5f88a301bb263b9134a02eb" +
  "440302eb440299884b1cb3f5d26f28bfa1f247339380b9f1000f08dbd5f8bc30" +
  "002bbfbf094b002203eb44039a800f2c05d197f8c8309e4201d000f0adf84046" +
  "bde8f88300462020fcae1b600050202010b5044600f00cf9024b2046db6abde8" +
  "10401847fcae1b6010b5044600f000f9024b20461b6bbde810401847fcae1b60" +
  "70b504460d4600f009f9034b294620465b6bbde870401847fcae1b6070b50446" +
  "0d4600f0fbf8034b294620469b6bbde870401847fcae1b6070b504460d4600f0" +
  "edf8034b29462046db6bbde870401847fcae1b6070b504460d4600f0dff8034b" +
  "294620461b6cbde870401847fcae1b6070b504460d4600f0d1f8034b29462046" +
  "5b6cbde870401847fcae1b60042902d0024b9b6c18478d20704700bffcae1b60" +
  "2de9f743dde90a45074690469946cde9004500f03df858b9074b42463846de6c" +
  "4b460b95b4460a9403b0bde8f0436047032003b0bde8f083fcae1b602de9f743" +
  "dde90a45074690469946cde9004500f01ff858b9074b424638461e6d4b460b95" +
  "b4460a9403b0bde8f0436047032003b0bde8f083fcae1b6001f040ba19460133" +
  "22f8021f102bfad1704700bf02502020f0b59df918600546059c002e39db03f0" +
  "fe0004f0f007384333d1032a28d9dbb240f20132e4b25b001f3302fb04330622" +
  "02fb06336a69d418d35c042b18d16378a3f16502242a13d8652b12d0662b14d0" +
  "0d4a3a20134413f8673c1a0903f00f0331323033487001208b7000230a70cb70" +
  "f0bd312253200023f4e7212231205323f0e70020f4e700bffcae1b600a4810b5" +
  "03680a4ca34204d0fff7a6ff00230460838490f8243033b1002380f82430bde8" +
  "1040fff799bf10bd00502020474d435410b50446fff7e2ff074894f82a301c1e" +
  "18bf012423b190f825300bb9fff784ff80f8254010bd00bf00502020074810b5" +
  "0368074ca34205d0fff776ff0023046080f82530012380f8243010bd00502020" +
  "474d435402120313230414243405152535450616263646560717273747576708" +
  "1828384858687800f53a0000f1c31b6059a80a60d9c20b605112000051120000" +
  "511200005112000051120000c9f20a60c9fd0a60c9fd0a6025642e25642e2564" +
  "2d63743100ffffff0023064a1946013302f8011f1174102bf9d1034b034a1a60" +
  "704700bf3350202030502020505248332de9f041124c16461f46124b884615e0" +
  "9a4201d0fff7e0ff0f2f0fd83c44237d012b0bd196f87330042b07d196f87450" +
  "653dedb2242d9cbf03232375064b324641461c683b46a446bde8f041604700bf" +
  "305020205052483360b01b600f292de9f34194bf6825002500900c4698bf4d43" +
  "0f29dff86080019105e00544b5f886309bb2012b07d0d8f8043021469847009b" +
  "02b0bde8f0810d4e0d4b32689a4201d0fff79aff3719012321463b75d8f80430" +
  "98473b7d032b05d195f89830b3fa83f35b093b71264400233375e0e730502020" +
  "5052483360b01b6081ab1b603d200000ffffffffffffffff3a464346dff800f0" +
  "89af1b60dff800f0b9ac1b60dff800f0e5af1b6025642e25642e25642d683034" +
  "00ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" +
  "ffffffffffffffff0023064a1946013302f8011f1174102bf9d1034b034a1a60" +
  "704700bf3350202030502020505248332de9f041124c16461f46124b22688846" +
  "9a4201d0fff7e0ff0f2f0fd83c44237d012b0bd196f87330042b07d196f87450" +
  "003dedb2892d9cbf03232375064b324641461c683b46a446bde8f041604700bf" +
  "305020205052483300c11b600f292de9f34194bf6825002500900c4698bf4d43" +
  "0f29dff86080019105d80544b5f886309bb2012b07d0d8f8043021469847009b" +
  "02b0bde8f0810d4e0d4b32689a4201d0fff79aff3719012321463b75d8f80430" +
  "98473b7d032b05d195f89830b3fa83f35b093b71264400233375e0e730502020" +
  "5052483300c11b607dc21b6051c31b60ffffffffffffffff3a464346dff800f0" +
  "29c01b60dff800f0b9ac1b60dff800f085c01b6025642e25642e25642d435432" +
  "00ffffffffffffff0f2b30b424d892f87340042c20d192f87440a4f18a05032d" +
  "1ad803f114010e4a885cc50703d5885c40f00400885400208c2c01d313b1013b" +
  "0433985cb0fa80f0400914f0010460404042400830bc7047dff808c030bc6047" +
  "3050202081ab1b600f2970b504460e461cd868234b438633c35a9bb2012b15d1" +
  "0e4d0f4b2a689a4202d195f824300bb10c4b984795f8543043b100220a4b0b49" +
  "00f046f903f8012b8b42fbd13146204642f23d03bde87040184700bf00502020" +
  "474d435475ae1b603450202044502020042b70b49df80c4018d1a4f18a06032e" +
  "14d80c4b760803eb860338bf01333ab11a44187810b10133934203d100230b70" +
  "70bc704701f8010bf3e70495039470bc00f010b874c21b6075c21b60034a0123" +
  "13700023024afef7b9bd0000545020200250202000000000f3f66cbc21505200" +
  "214e45002de9f0471c468846914600f0d9f80f2c51d899f87330042b4dd199f8" +
  "74708d2f49d804f11405284bea5cd6071fd5274a926d42b14a4508d192f87060" +
  "92f86f10f6b2090606d508260ee009f102018a42f9d1f1e792f88120002af4d1" +
  "2f2e25d8082c34bf18260826ea5c3243ea54a7f18c02012a1fd8ea5cd10703d5" +
  "ea5c42f02002ea54acb1114b013cb3f856309bb2234103f00103a7f18c025742" +
  "57419f4214bf6ff000400020bde8f087303e132ed7e72346efe723464a464146" +
  "dff810c0bde8f047604700bf305020200050202041c11b600f292de9f0410d46" +
  "dff88c8004d929464346bde8f0411847682606fb0106b6f886309bb2012bf2d1" +
  "00f060f8184c194b94f8551093f8c820914206d00022a4f8562093f8c83084f8" +
  "5530a76d2946a665c04705f1140edff844c01ef80c2013070dd5012303fa05f1" +
  "d506b4f8563002d596f8985055b123ea0101a4f8561000f023b90ef80c20a765" +
  "bde8f0811943f4e70050202000462020a1c11b60305020200f29104b2de9f041" +
  "05460e4602d9bde8f041184768244c43bc3407599847002f0ddb2b59002b0ada" +
  "00f010f8064a0123b2f85610b34021ea0301a2f85610bde8f08100bf11d00a60" +
  "00502020074b0849da6d8a4209d00022a3f856209a65054a92f8c82083f85520" +
  "d965704700502020484e45310046202085f85420a5f856207047ffff00000000" +
  "000004290ad1104601992de90c5000f017f8bde80c500421f2f694bef2f673be" +
  "642804d9662801d82730704702387047642804d98b2801d80230704727387047" +
  "8d2808b511d8fff7ebff43428b4208dcc0f18d038b4206db0844bde80840fff7" +
  "e7bf0020f9e78d20f7e708448d28a8bf8d2020eae07008bd042901d18d207047" +
  "f2f6e6be042906d1ddf818c0bcf1040f01d18d207047f2f6dbbeddf818c0bcf1" +
  "040f04d10eb5fff7c3ffbde80e40000000007047069a00f001b8000010b51446" +
  "43f6f5239847042c06d18d28034b28bf8d20bde81040184710bd00bfa9c41b60" +
  "042b73b50c461546dde9062103d1a2f16506282e09d90791214606922a46dff8" +
  "9cc002b0bde8704060471046204b9847892814d942081f4b03eb8203c20700d4" +
  "013355b125442146013b013da94202d013f8012f4abb00230b7002b070bd6528" +
  "08d145f23133adf8043000238df8063001abe6e7662802d10f4b0193f8e700f1" +
  "c04000f5dd1090f8953e1a0903f00f03313230338df804203a228df806300023" +
  "8df805208df80730e2e701f8012bcde7a9c41b6060c11b602131530049fb0a60" +
  "93060ad5530706d4aef11403d94001f001016344197142f0040202f00702fff7" +
  "ccbe";
export const recipe = Object.freeze({
  "base": 1610809344,
  "start": 1612426104,
  "end": 1612432954,
  "template_sha256": "bd12857664cf6b7617aedf0a2911257a9557b7e42cae1042de22ca4a9ab88ee0",
  "plans": [
    {
      "at": 1610810052,
      "hex": "10c11b6040fb01001c000000",
      "sha256": "466238b9428b1e2d2d67c63b55b71548d0bf74e53c45e42cb583453aa1de4947"
    },
    {
      "at": 1611311096,
      "hex": "10f146fa",
      "sha256": "3b790728310fd561277a99d6d2e9821bdbbe4d3de79605c111e129d415d4342a"
    },
    {
      "at": 1611318788,
      "hex": "d5ac1b60",
      "sha256": "bb62ee2fcd297583f8f98cff3458d5679771766467d3a08a062101c1fae06178"
    },
    {
      "at": 1611318932,
      "hex": "f1ac1b60",
      "sha256": "bb62ee2fcd297583f8f98cff3458d5679771766467d3a08a062101c1fae06178"
    },
    {
      "at": 1611320628,
      "hex": "0dad1b60",
      "sha256": "bb62ee2fcd297583f8f98cff3458d5679771766467d3a08a062101c1fae06178"
    },
    {
      "at": 1611322692,
      "hex": "29ad1b60",
      "sha256": "bb62ee2fcd297583f8f98cff3458d5679771766467d3a08a062101c1fae06178"
    },
    {
      "at": 1611329528,
      "hex": "0df198fa",
      "sha256": "e549e9783a26031b24c7ccad6adb7d4737494727803611d8ab40afeaa35e9d19"
    },
    {
      "at": 1611329636,
      "hex": "0df14afa",
      "sha256": "25f6815a867a02d98b3b59e58ffffa2d71c40b8d8cbd325e58c753600782aa6c"
    },
    {
      "at": 1611329912,
      "hex": "0df17cb900bf",
      "sha256": "11aa09712ee725b21ee85ddb1dd0ef7b7ed73f62c8d1df05077cccd18282d7dc"
    },
    {
      "at": 1611329970,
      "hex": "0bf1c7fd",
      "sha256": "c17863869c968f6aeabcc0b3b6c21ca0ccc487d5a95cbb53d42a89a4db868024"
    },
    {
      "at": 1611331630,
      "hex": "0cf165fe",
      "sha256": "5002e31013fc5ce2320acd937d0b9fdc31cad908e9d6db69d885273e7aa21962"
    },
    {
      "at": 1611331654,
      "hex": "0cf164fe",
      "sha256": "d6c1e07dae48d2f76d9e3234a994602b67b1bf874fdda2e17931192e0117da98"
    },
    {
      "at": 1611332424,
      "hex": "0cf15ebb",
      "sha256": "4322aebd453049637c3fab68fb3204fae6b3395f538c6355d0bfb22ea3507cb9"
    },
    {
      "at": 1611383418,
      "hex": "fef011fd",
      "sha256": "70b2bd880efda26f7c081fe258b91cd6b42f439aa5e01b9e3ac71b3e27198c7d"
    },
    {
      "at": 1611448220,
      "hex": "2cc11b60",
      "sha256": "2cfffb9ad2ab95c17c7d5f2f0813e059a8dce19f84d811454479030e09aa7041"
    },
    {
      "at": 1611528008,
      "hex": "dbf006fb",
      "sha256": "ceafcb91808f9809b8dfa3ca3a7912d0dd57f3e9dfc6c12689ec0fcb81a56c26"
    },
    {
      "at": 1611528060,
      "hex": "dbf00afb",
      "sha256": "83592ed2b5ab49f147f8e753017085e76e9ae147a93af0e1ca5228f72d5054b2"
    },
    {
      "at": 1611531194,
      "hex": "dcf099f8",
      "sha256": "0227d62adf762637796cdd1f9e6ed93ecc826ce30fb6b2911ffcd5e60e8c584a"
    },
    {
      "at": 1611531274,
      "hex": "dcf071f8",
      "sha256": "887f1168dd6d87f9733221c057938f8a14ff402a75e7f75256f9df7cf892a4f9"
    },
    {
      "at": 1611532086,
      "hex": "dbf0dbfe",
      "sha256": "70ccda111dba5f7ca0d576c50317b7662ceacd05c878ee8a6a70b584bc0f8512"
    },
    {
      "at": 1611532226,
      "hex": "dbf095fe",
      "sha256": "e46f27c3e25b3747d25f481099880c3573e3d41465c4775aad4d88ed09b85d2a"
    },
    {
      "at": 1611546482,
      "hex": "d8f0bdfa",
      "sha256": "bc6de1bf4b2fadae3da0a1c42f937c29dd03a22d6e6b7e7e112a7a388dc68c66"
    },
    {
      "at": 1611546576,
      "hex": "d8f0c2fa",
      "sha256": "bc42291dd006503c32c1166f7fde5ac69e65d0517f863404f746a0898c17d270"
    },
    {
      "at": 1611547134,
      "hex": "d8f077f9",
      "sha256": "f5351e3ec5b5a7e301dc487a20985b1d931422752f837149257712c9952b9dac"
    },
    {
      "at": 1611547228,
      "hex": "d8f07cf9",
      "sha256": "d634365dc24b6374612b14dd684bbfb5c8f0f67461deea1b62af77a8658a60dc"
    },
    {
      "at": 1612253106,
      "hex": "1cf037fd",
      "sha256": "7ff8dd2ccec0e3f9fc746d1e4c662a28ed55fbfb6a0cf68a09028a994598a62f"
    },
    {
      "at": 1612254734,
      "hex": "1cf013fa",
      "sha256": "38a7f39714b72beff0ecbc9896bda440523de64cbffb716145c28e216892f395"
    },
    {
      "at": 1612254774,
      "hex": "1cf0fbf9",
      "sha256": "2a2afc749e3fd6c7603bacba538b8f5481f5c1033abed869a65a4c815f9fac0d"
    },
    {
      "at": 1612369664,
      "hex": "fdab1b60",
      "sha256": "7573570d4aa3d074a8662d0fa56eb87b4073ed37c96a1b1991bddf5f60267243"
    }
  ],
  "copies": [
    {
      "from": 1611332424,
      "to": 1612431980,
      "length": 4,
      "sha256": "4322aebd453049637c3fab68fb3204fae6b3395f538c6355d0bfb22ea3507cb9"
    },
    {
      "from": 1611329912,
      "to": 1612432500,
      "length": 6,
      "sha256": "11aa09712ee725b21ee85ddb1dd0ef7b7ed73f62c8d1df05077cccd18282d7dc"
    },
    {
      "from": 1611331654,
      "to": 1612432678,
      "length": 4,
      "sha256": "d6c1e07dae48d2f76d9e3234a994602b67b1bf874fdda2e17931192e0117da98"
    }
  ],
  "record_ends": [
    1612427417,
    1612431673,
    1612431992,
    1612431996,
    1612432498,
    1612432624,
    1612432920,
    1612432954
  ]
});
export function buildProgram() { return Uint8Array.from(hex.match(/../g), s => parseInt(s, 16)); }
