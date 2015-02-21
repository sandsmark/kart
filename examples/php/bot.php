<?php
define(INPUT_UP, 1<<0);
define(INPUT_DOWN, 1<<1);
define(INPUT_LEFT, 1<<2);
define(INPUT_RIGHT, 1<<3);
define(INPUT_SPACE, 1<<4);
define(INPUT_RETURN, 1<<5);

function composeCommand($commands) {
    if(!is_array($commands)) {
        $commands = array($commands);
    }

    $cmd = 0;
    foreach($commands as $command) {
        $cmd |= $command;
    }

    return strval($cmd);
}

function sendLine($sock, $line) {
    fputs($sock, $line."\n");
}

function readPacket($sock) {
    return json_decode(fgets($sock));
}

// Connect to the server
$sock = fsockopen("localhost", 31337);
socket_set_timeout($sock, 0, 250000);

sendLine($sock, "TestBot");

// Read "round start" packet, this one differs from all the consecutive packets. Contains map, my id, tile size and so on...
$round_data = readPacket($sock);

// Extract some data from the first round packet
$me = $round_data->id;
$car_count = $round_data->num_cars;

// Get some of the round data and store it in a handy variable
$map = $round_data->map->tiles;
$modifiers = $round_data->map->modifiers;
$checkpoints = $round_data->map->path;

// Store away tile size
$tilesize = (object)array( "width" => $round_data->map->tile_width, "height" => $round_data->map->tile_height );

// Print some of the values to see that we get proper data
print "I am ID ".$me." and there are ".$car_count." other cars\n";
print "Tiles are ".$tilesize->width."x".$tilesize->height."\n";

// Send the first command. Since the server responds with a status packet each time we send a command we can 
// kick things off by pressing the accelerator
sendLine($sock, composeCommand(array(INPUT_UP)));

$frame = 0;

// Keep looking at the status packet and sending commands
while(true) {
    $frame++;
    $status = readPacket($sock);

    if(is_object($status)) {
        foreach($status->cars as $car) {
            if($car->id == $me) {
                $mystatus = $car;
                break;
            }
        }
        
        if($frame % 25 == 0) {
            // Lets print what is going on...
            print "Position: ".intval($mystatus->pos->x)."x".intval($mystatus->pos->y)."\t";
            print "Velocity: ".intval($mystatus->velocity->x)."x".intval($mystatus->velocity->y)."\r";      
        }
    }

    $cmd = array();

    // Lets just do something random
    $decision = rand(0, 100);

    // Never make decisions based on one test map, you will always be owned by the crew making silly maps
    // for the actual compo.
    if($decision > 60) {
        $cmd[] = INPUT_UP;

    } elseif($decision < 10) {
        $cmd[] = INPUT_RIGHT;

    } elseif($decision > 47 && $decision < 52) {
        $cmd[] = INPUT_RETURN;
    }
    
    sendLine($sock, composeCommand($cmd));
}

fclose($sock);
?>
