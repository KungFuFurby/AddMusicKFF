var attack = 4, decay = 2, sustain = 5, release = 9;
var scale = 128;
var CLOCKS = [
    0, 2048, 1536, 1280, 1024, 768, 640, 512,
  384,  320,  256,  192,  160, 128,  96,  80,
   64,   48,   40,   32,   24,  20,  16,  12,
   10,    8,    6,    5,    4,   3,   2,   1,
];
var ADSR_WIDTH = 800;
var ADSR_HEIGHT = 400;

function init()
{
    setAttack(attack);
    setDecay(decay);
    setSustain(sustain);
    setRelease(release);

    for (var j = 0; j < 0x10; j++)
        document.getElementById("a" + j).innerHTML = getAttackTime(j).toFixed(3);
}

function setAttack(id)
{
    attack = id;
    draw();
    highlightCell("a", id);
}

function setDecay(id)
{
    decay = id;
    draw();
    highlightCell("d", id);
}

function setSustain(id)
{
    sustain = id;
    draw();
    highlightCell("s", id);

    for (var j = 0; j < 0x08; j++)
        document.getElementById("d" + j).innerHTML = getDecayTime(j).toFixed(3);
    for (var j = 0; j < 0x20; j++)
        document.getElementById("r" + j).innerHTML = getReleaseTime(j).toFixed(3);
}

function setRelease(id)
{
    release = id;
    draw();
    highlightCell("r", id);
}

function getAttackTime(id)
{
    return id == 31 ? 0 : CLOCKS[id * 2 + 1] * 64 / 32000;
}

function getDecayTime(id)
{
    return CLOCKS[id * 2 + 0x10] * [440, 312, 227, 163, 112, 69, 32, 0][sustain] / 32000;
}

function getSustainLevel()
{
    return (sustain + 1) / 8;
}

function getReleaseTime(id)
{
    return id == 0 ? Infinity : CLOCKS[id] * [255, 383, 469, 533, 584, 626, 663, 695][sustain] / 32000;
}

function makeEnvelopeADSR()
{
    var t = 0;
    var z = 0;
    var env = [[t, z]];
    while (z < 0x7FF)
        env.push([t += CLOCKS[attack * 2 + 1], z += Math.min(attack == 0x1F ? 0x400 : 0x20, 0x7FF - z)]);
    while (z >= 0x100 * (sustain + 1))
        env.push([t += CLOCKS[decay * 2 + 0x10], (--z, z -= z >> 8)]);
    if (release != 0)
        while (z > 0)
            env.push([t += CLOCKS[release], (--z, z -= z >> 8)]);

    return env;
}

function zoomOut(s)
{
    scale /= 2;
    draw();
}

function zoomIn()
{
    scale *= 2;
    draw();
}

function highlightCell(kind, id)
{
    var lim = {a: 0x10, d: 0x8, s: 0x8, r: 0x20}[kind];
    for (var j = 0; j < lim; j++)
    {
        document.getElementById(kind + j).style.backgroundColor="transparent";
        document.getElementById(kind + j).style.color="#FFFFFF";
    }
    document.getElementById(kind + id).style.backgroundColor="#55b0ff";
    document.getElementById(kind + id).style.color="#000000";
}

function roundPrec(num, dec)
{
    return Math.round(num * Math.pow(10, dec)) / Math.pow(10, dec);
}

function strpad(str, width)
{
    if (str.length < width)
    {
        var newstr = "";
        for (var i = str.length; i < width; i++)
            newstr += "0";
        str = newstr + str;
    }
    return str;
}

function draw()
{
    var c = document.getElementById("ADSRCanvas");
    c.width = ADSR_WIDTH;
    var ctx = c.getContext("2d");
    ctx.fillStyle = "rgb(255, 255, 255)";
    ctx.fillRect(0, 0, ADSR_WIDTH, ADSR_HEIGHT);

    //var scale = 800.0 / (getAttackTime(attack) + getDecayTime(decay) +
    //	(release != 0 ? getReleaseTime(release) : 38));

    var env = makeEnvelopeADSR();
    var e0 = env.shift();

    ctx.strokeStyle = 'black';
    ctx.lineWidth = 10;
    ctx.beginPath();
    ctx.moveTo(scale * e0[0] / 32000, ADSR_HEIGHT * (1 - e0[1] / 0x7FF));
    for (var e of env)
        ctx.lineTo(scale * e[0] / 32000, ADSR_HEIGHT * (1 - e[1] / 0x7FF));
    var e1 = env.pop();
    if (release == 0)
        ctx.lineTo(ADSR_WIDTH, ADSR_HEIGHT * (1 - e1[1] / 0x7FF));
    else
        ctx.lineTo(scale * e1[0] / 32000, ADSR_HEIGHT + ctx.lineWidth);
    ctx.stroke();

    ctx.lineWidth = 1;
    ctx.font = "12px Arial";

    for (var i = 0; i <= ADSR_WIDTH; i += 32)
    {
        if (i == 0)
            ctx.textAlign = "left";
        else if (i == ADSR_WIDTH)
            ctx.textAlign = "right";
        else
            ctx.textAlign = "center";
        ctx.beginPath();
        ctx.moveTo(i, 400);
        ctx.lineTo(i, 390);
        ctx.stroke();
        ctx.fillStyle = "Black";
        ctx.fillText(roundPrec(i/scale, 2).toString(), i, 370, 24);
    }

    var t1 = getAttackTime(attack);
    var t2 = t1 + getDecayTime(decay);

    ctx.strokeStyle = "rgba(0, 0, 0, 0.25)";
    ctx.beginPath();
    ctx.moveTo(scale * t1, 0);
    ctx.lineTo(scale * t1, ADSR_HEIGHT);
    ctx.moveTo(scale * t2, ADSR_HEIGHT * (1 - getSustainLevel()));
    ctx.lineTo(scale * t2, ADSR_HEIGHT);
    for (var i = 1; i <= 7; ++i) {
        ctx.moveTo(0, i / 8 * ADSR_HEIGHT);
        ctx.lineTo(ADSR_WIDTH, i / 8 * ADSR_HEIGHT);
    }
    ctx.stroke();

    document.getElementById("ADSRLabel").innerHTML =
        "<tr><td>ADSR Command</td><td> $ED $" +
        strpad((decay * 16 + attack).toString(16).toUpperCase(), 2) + " $" + strpad((sustain * 32 + release).toString(16).toUpperCase(), 2) +
        "</td><tr><td>Instrument Syntax</td><td>\"SampleName.brr\" $" +
        strpad(((decay * 16 | 0x80) + attack).toString(16).toUpperCase(), 2) + " $" + strpad((sustain * 32 + release).toString(16).toUpperCase(), 2) + " $00 $XX $xx</td>";
}
