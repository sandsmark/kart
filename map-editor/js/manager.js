function Manager(size, inputManager) {
    this.size = size; // Size of the grid

    this.setup();
}

// Set up the game
Manager.prototype.setup = function () {
    this.grid = new Grid(this.size);
};