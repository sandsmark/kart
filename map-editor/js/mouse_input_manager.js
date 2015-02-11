function InputManager(container) {
    this.size = size;

    $(container).on('click', function(e){
        var tile = calculateTile({x: event.offsetX, y: event.offsetY}, container);

        $('#kart').trigger('cycleTile', [tile]);
    });

    var calculateTile = function(position, container) {

        // Corrects coordinates according to canvas' size
        var rect = container.getBoundingClientRect();
        position.x -= rect.left;
        position.y -= rect.top;

        var squareX = Math.floor(position.x / container.width * this.size);
        var squareY = Math.floor(position.y / container.height * this.size);
        return {x: squareX, y: squareY};
    };
}