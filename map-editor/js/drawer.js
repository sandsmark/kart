var drawer;
var size = 10;

function Drawer(canvas) {
    // Public vars
    this.tiles = size;

    this.canvas = canvas;
    this.ctx = this.canvas.getContext('2d');

    this.spacing = 2;
    this.h = this.canvas.width / this.tiles - this.spacing;
    this.w = this.canvas.width / this.tiles - this.spacing;

    var inputManager = new InputManager(canvas);
    this.manager = new Manager({x:this.tiles, y:this.tiles}, inputManager);

    this.draw();

    /**
     * Set up events
     */
    $("#kart").on("draw", function() {
        drawer.draw();
    });

    $('#kart').on('cycleTile', function(e, tile) {
        drawer.manager.grid.cycleTile(tile);
        drawer.draw();
    });
}

Drawer.prototype.draw = function() {
    this.ctx.clearRect(0, 0, this.canvas.width, this.canvas.height);

    for (var x = 0; x < this.manager.grid.size.x; x++) {
        for (var y = 0; y < this.manager.grid.size.y; y++) {
            var cell = this.manager.grid.cells[x][y];
            this.ctx.drawImage(cell.tex, x * this.w + (x * this.spacing), y * this.h + (y * this.spacing), this.w, this.h);
        }
    }
}
