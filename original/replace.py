import re

with open('original.js') as original_f:
    original = original_f.read()

items = ["clustersCanvas","getElementById","2d","getContext","width","height","x","y","w","h","force","radius","steps","color","rgb( 127, 127, 127 )","type","xv","yv","xf","yf","timer","clusters.update()","createButtons","randomizeGenes","random","floor","initialize","initializeRandomWholeScreen","rgb( 200,  30,  30 )","rgb( 200, 150,  30 )","rgb( 200, 200,  30 )","rgb(  30, 190,  30 )","rgb(  70,  70, 210 )","rgb( 130,  40, 170 )","rgb( 110,  30,  30 )","rgb( 110,  80,  30 )","rgb( 120, 120,  50 )","rgb(  30, 100,  30 )","rgb(  30,  30, 160 )","rgb( 100,  30, 130 )","loadGenes","renderButtons","update","updateParticleInteractions","updateWallCollisions","sqrt","render","fillStyle","rgb(  70,  80,  90 )","strokeStyle","rgb( 120, 140, 160 )","fillRect","strokeRect","font","15px sans-serif","textBaseline","top","rgb( 160, 160, 160 )","Randomize","fillText","Gems","Alliances","Red Menace","Acrobats","Mitosis","Planets","Ships","rgba( 0, 0, 0, "," )","showParticle","lineWidth","rgb( 100, 100, 100 )","beginPath","PI","arc","stroke","fill","closePath","mouseDown","bigBang","mouseMove","mouseUp","printGenes","type[","].force [ ","] = ",";","log","].radius[ ","].steps = ","initializeRandomOnBottom","initializeWithStripes","initializeAsDisk","setPlanetValues","initializeAsIsolatedBlobs","mutate","clearVelocities","sin","cos","arrowRight","onkeydown","keyCode","arrowLeft","onmousedown","pageX","pageY","onmousemove","onmouseup"]
replaced = original
for i, item in enumerate(items):
    if re.match('^[A-Za-z]+$', item):
        replaced = replaced.replace(f'[_0x13d4[{i}]]', f'.{item}')
    else:
        replaced = replaced.replace(f'_0x13d4[{i}]', f'"{item}"')

with open('replaced_strings.js', 'w') as replaced_f:
    replaced_f.write(replaced)
