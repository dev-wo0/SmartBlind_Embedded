var http = require('http');
var url = require('url');
var fs = require('fs');

var arduinoFileName = "/tmp/arduino.txt";


function sendResponse(BlindOn, AutoOn, SleepOn, Alarm, remoteIP, response) {

    if (remoteIP != null){
    	    console.log('\nRequest to Blind ' + (BlindOn?'Up':'Down'));
    	    console.log('from ' + remoteIP);
    	    var fileStream = fs.createWriteStream(arduinoFileName);
    	    fileStream.write(BlindOn + "\n" + AutoOn + "\n" + SleepOn + "\n" + Alarm + "\n" + remoteIP + "\nOK\n");
    	    fileStream.end();

    	    response.writeHead(302, {
    	    		    'Location': '/'
    	    });
    	    response.end();
    } else {
     response.writeHead(200, { 'Content-Type': 'text/html' });
     response.write('<!DOCTYPE html><html lang="en"><head>');
     response.write('<meta charset="utf-8">');
     response.write('<meta http-equiv="refresh" content="30" />');
     response.write('<title>BLIND switch</title>');
     if (!BlindOn) {
      response.write ('<style>body{background-color:black;color:white;}</style>');
     }

     response.write('</head>');
     response.write('<body><h1 style="margin-top: 10%;">Auto Mode</h1>');

     response.write('</h1>');
     response.write('<a href="/auto">Start!</a>');

     response.write('<body><h1>HandMode'+(BlindOn?' Blind is Up':' Blind is Down'));

     response.write('</h1>');

     if (BlindOn){
      response.write('<a href="/down">Blind down!</a>');
     } else {
      response.write('<a href="/up">Blind up!</a>');
     }
     response.write('<body><h1>SleepMode');
     response.write('</h1>');

     response.write('<input type="textarea" cols="5" rows="1" id="secondtext"></>');
     //response.write('</textarea>');

     response.write('<a href="/sleep">Start!</a>');
     response.write('</body></html>');
     response.end();
    }
}


function processRequest(request, response) {
    "use strict";
    var pathName = url.parse(request.url).pathname;
    var second = 10;


    var remoteIP = request.headers['X-Forwarded-For'] == undefined?request.connection.remoteAddress:request.headers['X-Forwarded-For'];

    if (pathName == "/up") {
     sendResponse (true, false, false, null, remoteIP, response);
   } else if (pathName == "/down") {
     sendResponse (false, false, false, null, remoteIP, response);
   }  else if (pathName == "/auto") {
     sendResponse (true, true, false, null, remoteIP, response);
   }  else if (pathName == "/sleep") {
     sendResponse (false, false, true, second, remoteIP, response);
   }  else if (pathName == "/") {
     fs.exists(arduinoFileName, function (exists) {
    	    if (exists){
    	    	fs.readFile(arduinoFileName,  function(err,data){
    	    	    if (!err){
    	    	     console.log('\nStored data:\n' + data);
    	    	     var storedData = data.toString().split("\n");
		     sendResponse ((storedData[0] == "true"), false, false, null, null, response);
    	    	    }else{
    	    	     console.log(err);
    	    	    }
    	    	});
    	    } else {
    	       sendResponse (false, false, false, null, null, response);
    	    }
    	 });
    } else {
    	 response.end();
    }
}

http.createServer(processRequest).listen(8080);

console.log("Server running at port 8080");
