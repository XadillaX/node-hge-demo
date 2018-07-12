const hge = require("../build/Release/node_hge.node");

process.stdin.resume();

let time = 0;

hge.setFrameCallback(function(dt) {
    time += dt;
    if(time > 5) time -= 5;
    if(time < 0) time = -time;
    console.log("FPS:", 1 / dt);
});

hge.setRenderCallback(function() {
    const ret = `ff${parseInt((255 * ((5 - time) / 5))).toString(16)}0000`;
    console.log(ret);
    hge.gfxClear(parseInt(ret, 16));
});

hge.setHGEDoneCallback(function() {
    console.log("Bye!");
    process.exit();
});

hge.start();