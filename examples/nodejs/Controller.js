NavigatorHandler = require('./Navigator.js');

/**
 * Constructor
 * @param socket
 */
function Controller(socket)
{
    this.socket = socket;

    // Store the Navigator properties
    this.navigator;

    // Updated each iteration
    this.players = [];
    this.shells = [];
    this.boxes = [];
}

/**
 * Initialize all values
 * @param data
 */
Controller.prototype.init = function(data) {
    // Initialize our GPS
    this.navigator = new NavigatorHandler(data);

    // Let the server know we're ready
    this.write(1<<5);
};

/**
 * Update values and start doing fun stuff
 * @param data
 */
Controller.prototype.update = function(data) {
    // Update all the useful parameters
    this.players = data.cars;
    this.shells = data.shells;
    this.boxes = data.boxes;

    // Send a random direction to the server
    this.write(this.navigator.randomDirection());
};

/**
 * Write input to server
 * @param input
 */
Controller.prototype.write = function(input)
{
    this.socket.write(input +"\n")
};

// Export module for external usage
module.exports = Controller;