var addon   = require('neurose');

console.log(addon.connect_redis("127.0.0.1",6379));
console.log(addon.ping_redis());

var body =
"{   \"id\" : \"b8d36032-42d9-11e6-9d7c-f04da2e5162d\"," +
"   \"number\" : \"1\"," +
"   \"reference\" : \"x\"," +
"   \"status\" : \"CLOSED\"," +
"   \"created_at\" : \"2016-07-05 15:40:14.010\"," +
"   \"updated_at\" : \"2016-07-05T15:40:14.055Z\"," +
"   \"items\" : [" +
"      { " +
"         \"sku\" : \"b8d36032-42d9-11e6-9d7c-f04da2e5162d\"," +
"         \"unit_price\" : 1000," +
"         \"quantity\" : 10" +
"      }  " +
"   ] ," +
"   \"notes\" : \"\"," +
"   \"transactions\" : [" +
"      { " +
"         \"id\" : \"b8d36032-42d9-11e6-9d7c-f04da2e5162d\"," +
"         \"external_id\" : \"sbrubles\"," +
"         \"amount\" : 1000," +
"         \"type\" : \"PAYMENT|CANCEL\"," +
"         \"authorization_code\" : \"94721719\"," +
"         \"card_brand\" : \"AMEX\" ," +
"         \"card_bin\" : \"710\"," +
"         \"card_last\" : \"4749\"" +
"      } " +
"   ]," +
"   \"price\" : 1000}"
;

var method = "GET";
var url = "/orders";
console.log(addon.work(method,url,body,1));

url = "/orders?number=1";
console.log(addon.work(method,url,body,1));

url = "/orders/b8d36032-42d9-11e6-9d7c-f04da2e5162d";
console.log(addon.work(method,url,body,1));

url = "/orders/b8d36032-42d9-11e6-9d7c-f04da2e5162d/items";
console.log(addon.work(method,url,body,1));

url = "/orders/b8d36032-42d9-11e6-9d7c-f04da2e5162d/transactions";
console.log(addon.work(method,url,body,1));


method="POST"
var url = "/orders";
console.log(addon.work(method,url,body,1));

method="PUT"
var url = "/orders";
console.log(addon.work(method,url,body,1));

var url = "/orders/b8d36032-42d9-11e6-9d7c-f04da2e5162d";
console.log(addon.work(method,url,body,1));

var url = "/orders/b8d36032-42d9-11e6-9d7c-f04da2e5162d/transactions/c8d36032-42d9-11e6-9d7c-f04da2e5162d";
console.log(addon.work(method,url,body,1));


//method="DELETE"
//var url = "/orders";
//console.log(addon.work(method,url,body,1));
console.log(addon.disconnect_redis());

