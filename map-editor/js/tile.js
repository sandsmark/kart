/**
 * Create the Tile object
 * @param position
 * @param intValue
 * @constructor
 */
function Tile(position, intValue) {
    this.intValue = intValue || 0;
    this.x = position.x;
    this.y = position.y;

    this.char = map[this.intValue].char;
    this.tex = map[this.intValue].tex;
    this.name = map[this.intValue].name;
}

/**
 * Cycle a tile one step
 */
Tile.prototype.cycle = function() {
    this.intValue = (this.intValue += 1) % 7;
    this.char = map[this.intValue].char;
    this.tex = map[this.intValue].tex;
    console.log(this);
};


/**
 * Image factory
 * @param src
 * @param title
 * @returns {Image}
 */
Tile.createImage = function(src, title) {
    var img   = new Image();
    img.src   = src;
    img.alt   = title;
    img.title = title;
    return img;
};

/**
 * Object containing all textures and information
 * @type {{name: 'string, char: string', tex: Image}[]}
 */
var map =
    [
        {
            name: 'map_grass',
            char: '*',
            tex: Tile.createImage('img/map-none.bmp', 'map_grass')
        },
        {
            name: 'map_ horizontal',
            char: '-',
            tex: Tile.createImage('img/map-horizontal.bmp', 'map_horizontal')
        },
        {
            name: 'map_vertical',
            char: '|',
            tex: Tile.createImage('img/map-vertical.bmp', 'map_vertical')
        },
        {
            name: 'map_topleft',
            char: '/',
            tex: Tile.createImage('img/map-topleft.bmp', 'map_topleft')
        },
        {
            name: 'map_topright',
            char: '`',
            tex: Tile.createImage('img/map-topright.bmp', 'map_topright')
        },
        {
            name: 'map_bottomright',
            char: ',',
            tex: Tile.createImage('img/map-bottomright.bmp', 'map_bottomright')
        },
        {
            name: 'map_bottomleft',
            char: '\\',
            tex: Tile.createImage('img/map-bottomleft.bmp', 'map_bottomleft')
        }
    ];


