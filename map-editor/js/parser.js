var parser = {
    gridToText: function(grid) {
        $('textarea').val(grid.size.x +'x'+ grid.size.y +'\n');
        for (var x = 0; x < grid.size.x; x++) {
            var line = '';
            for (var y = 0; y < grid.size.y; y++) {
                line += grid.cells[y][x].char;
            }
            $('textarea').val($('textarea').val()+line+'\n');
        }
    }
}