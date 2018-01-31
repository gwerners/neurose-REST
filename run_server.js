//https://nodejs.org/docs/v0.6.0/api/cluster.html
//http://goldfirestudios.com/blog/136/Horizontally-Scaling-Node.js-and-WebSockets-with-Redis
var cluster = require('cluster');
var http    = require('http');
var addon   = require('neurose');

var numCPUs = require('os').cpus().length;
var numReqs = 0;
var numReqsTotal =0;
const PORT=9085;
const neurose_body =
"{   \"id\" : \"b8d36032-42d9-11e6-9d7c-f04da2e5162d\","
"   \"number\" : \"1\","
"   \"reference\" : \"x\","
"   \"status\" : \"CLOSED\","
"   \"created_at\" : \"2016-07-05 15:40:14.010\","
"   \"updated_at\" : \"2016-07-05T15:40:14.055Z\","
"   \"items\" : ["
"      { "
"         \"sku\" : \"b8d36032-42d9-11e6-9d7c-f04da2e5162d\","
"         \"unit_price\" : 1000,"
"         \"quantity\" : 10"
"      }  "
"   ] ,"
"   \"notes\" : \"\","
"   \"transactions\" : ["
"      { "
"         \"id\" : \"b8d36032-42d9-11e6-9d7c-f04da2e5162d\","
"         \"external_id\" : \"sbrubles\","
"         \"amount\" : 1000,"
"         \"type\" : \"PAYMENT|CANCEL\","
"         \"authorization_code\" : \"94721719\","
"         \"card_brand\" : \"AMEX\" ,"
"         \"card_bin\" : \"710\","
"         \"card_last\" : \"4749\""
"      } "
"   ],"
"   \"price\" : 1000}"
;

function handleRequest(request, response){
   var headers = request.headers;
   var method = request.method;
   var url = request.url;
   var body = [];
   request.on('error', function(err) {
      console.error(err);
   }).on('data', function(chunk) {
      body.push(chunk);
   }).on('end', function() {
      body = Buffer.concat(body).toString();


      //resposta dummy para testar o server http do node:
      /*response.end(
		  "<head><title>GWF Basic Page</title></head>"
	  	+ "<body>"
		+ "<h2>method  "+request.method +"</h2><br>"
		+ "<h2>url "+ request.url +"</h2><br>"
		+ "</body>");*/

	   //resposta tratada do modulo:
       //response.end(addon.work(request.method,request.url,body));
	   response.end(addon.work(request.method,request.url,neurose_body));
       
	   //notifica processo pai que o request foi tratado:
	   process.send({ cmd: 'notifyRequest' });
   });
}
function handleInterval(){
   console.log("Requests: Total =[%d] Last[%d]",numReqsTotal, numReqs);
   numReqsTotal += numReqs;
   numReqs =0;
}
if (cluster.isMaster) {

   // Fork workers.
   console.log("will fork %d workers",numCPUs)
   for (var i = 0; i < numCPUs; i++) {
      var worker = cluster.fork();

      worker.on('message', function(msg) {
         if (msg.cmd && msg.cmd == 'notifyRequest') {
            numReqs++;
         }
      });
   }
   console.log('server pid is ' + process.pid)
   cluster.on('death', function(worker) {
      console.log('worker ' + worker.pid + ' died. restart...');
      cluster.fork();
   });


   setInterval(handleInterval, 5000);
} else {


   // Worker processes have a http server.
   //http.Server(handleRequest).listen(PORT);
   http.Server(handleRequest).listen(PORT, function(){
   //Callback triggered when server is successfully listening. Hurray!
   console.log("Worker listening on: http://localhost:%s", PORT);
   console.log('worker pid is ' + process.pid)

});

}
