// Require required files
var net = require('net');
var GameController = require('./Controller.js');

// Initialize the net socket
var HOST = 'localhost';
var PORT = 31337;
var client = new net.Socket();

// Initialize the Controller
var Controller = new GameController(client);

// Iteration counter
var iter = 0

/**
 * Connects to the server on given port and host
 */
function Connect()
{
    // Connect the client to the game server
    client.connect(PORT, HOST);
}

// Call connect upon starting
Connect();

/**
 * Function called when a connection is successfully established
 */
client.on('connect', function()
{
    // Say something useful
    console.log('CONNECTED TO: ' + HOST + ':' + PORT);
});

/**
 * Function called upon receiving data
 */
client.on('data', function(data) {
    // Start a small timer
    var hrstart = process.hrtime();

    iter++;
    console.log("--------Iteration : " + iter + "------");

    // Parse the data into a usable JSON object
    var parsedData = JSON.parse(data.toString("utf-8"));

    // First time running
    if (iter == 1) {
        Controller.init(parsedData);
    } else {
        Controller.update(parsedData);
    }

    // Stop the timer and post execution time of the function
    var hrend = process.hrtime(hrstart);
    console.info("Execution time: %ds %dms", hrend[0], hrend[1]/1000000);
    console.log("--------------------------------------\n");
});

/**
 * Function called if a connection fails
 */
client.on('error', function(data) {
    // Say something useful
    console.log('ERROR: ' + data);

    // Destroy the client to ensure properly killing the process
    client.destroy();
});

/**
 * Function called if e.g. server disconnects
 */
client.on('close', function(error) {
    if (error == true)
    {
        // Post a somewhat useful error message
        console.log("Unexpected disconnect");
    } else {
        // First try a reconnect
        console.log("Disconnected. Trying reconnect in 1 second");

        // Try to reconnect once in 1 second
        setTimeout(function()
        {
            Connect();
        } , 1000);
        client.destroy();
    }
});

