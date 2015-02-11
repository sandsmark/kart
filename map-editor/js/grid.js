function Grid(size)
{
    this.size = size;
    this.cells = this.empty();
}

Grid.prototype.empty = function() {
    var cells = [];

    for (var x = 0; x < this.size.x; x++)
    {
        var row = cells[x] = [];

        for (var y = 0; y < this.size.y; y++)
        {
            row.push(new Tile({x:x, y:y}));
        }
    }

    return cells;
};

Grid.prototype.changeSize = function(size) {
    if (size.x != this.size.x) {
        this.addRow(size.x - this.size.x);
    } else if (size.y != this.size.y) {
        this.addColumn(size.y - this.size.y);
    }
};

Grid.prototype.cycleTile = function(coords) {
    var tile = this.cells[coords.x][coords.y];
    tile.cycle();
}

Grid.prototype.addRow = function() {

};

Grid.prototype.addColumn = function() {

};

Grid.prototype.serialize = function() {

};