/**
 * Define possible server commands
 * @type {number}
 */
const NET_INPUT_UP = 1<<0;
const NET_INPUT_DOWN = 1<<1;
const NET_INPUT_LEFT = 1<<2;
const NET_INPUT_RIGHT = 1<<3;
const NET_INPUT_SPACE = 1<<4;
const NET_INPUT_RETURN = 1<<5;

/**
 * Constructor
 * @param mapData
 */
function navigator(mapData) {
    this.map = mapData;

}

/**
 * Get a random direction
 */
navigator.prototype.randomDirection = function() {
    return this.returnAction(Math.floor(Math.random() * 4));
};

/**
 * Return the direction which a number represents
 * @param input
 * @returns {number}
 */
navigator.prototype.returnAction = function(input) {
    switch(input) {
        case 0:
            return NET_INPUT_UP;
        case 1:
            return NET_INPUT_DOWN;
        case 2:
            return NET_INPUT_LEFT;
        case 3:
            return NET_INPUT_RIGHT;
        case 4:
            return NET_INPUT_SPACE;
        case 5:
            return NET_INPUT_RETURN;
        default:
            return NET_INPUT_UP;
    }
};

// Export module for external usage
module.exports = navigator;