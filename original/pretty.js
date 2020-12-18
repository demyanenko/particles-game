var canvasID = document.getElementById("clustersCanvas");
var canvas = canvasID.getContext("2d");
function Clusters() {
    var _friction = 0.2;
    var _maxForce = 4.0;
    var _mouseForce = 0.4;
    var _particleRadius = 2.7;
    var _repelRadius = 1.6 * _particleRadius;
    var _maxInteractionRadius = 30.0 * _particleRadius;
    var _mouseGrabRadius = 30.0;
    var _particleCount = 600;
    var _maxSteps = 4;
    var _buttonWidth = 110;
    var _buttonHeight = 30;
    var _buttonGemsIndex = 0;
    var _buttonAlliancesIndex = 1;
    var _buttonRedMenaceIndex = 2;
    var _buttonAcrobatsIndex = 3;
    var _buttonMitosisIndex = 4;
    var _buttonPlanetsIndex = 5;
    var _buttonShipsIndex = 6;
    var _buttonRandomizeIndex = 7;
    var _buttonCount = 8;
    var _defaultGenesIndex = _buttonShipsIndex + 1;
    var _particleTypeCount = 12;
    var _updatePeriodMs = 10;
    var _backgroundTransparency = 0.7;
    var _width = canvasID.width;
    var _height = canvasID.height - _buttonHeight - 1;
    function _rect() {
        this.x = 0;
        this.y = 0;
        this.w = 0;
        this.h = 0
    }
    function _particleType() {
        this.force = new Array(_particleTypeCount);
        this.radius = new Array(_particleTypeCount);
        this.steps = 0;
        this.color = "rgb( 127, 127, 127 )"
    }
    function _particle() {
        this.type = 0;
        this.x = 0.0;
        this.y = 0.0;
        this.xv = 0.0;
        this.yv = 0.0;
        this.xf = new Array(_maxSteps);
        this.yf = new Array(_maxSteps)
    }
    var _buttons = new Array(_rect);
    var _particles = new Array(_particleCount);
    var _particleTypes = new Array(_particleTypeCount);
    var _isMouseDown = false;
    var _mouseX = 0;
    var _mouseY = 0;
    var _prevMouseX = 0;
    var _prevMouseY = 0;
    this.timer = setTimeout("clusters.update()", _updatePeriodMs);
    this.createButtons = function() {
        for (var i = 0; i < _buttonCount; i++) {
            _buttons[i] = new _rect();
            _buttons[i].w = _buttonWidth;
            _buttons[i].h = _buttonHeight;
            _buttons[i].x = 1 + i * ((_width - 2) / _buttonCount);
            _buttons[i].y = _height - 1
        }
    }
    ;
    this.randomizeGenes = function() {
        for (var i = 0; i < _particleTypeCount; i++) {
            _particleTypes[i].steps = Math.floor(_maxSteps * Math.random());
            for (var j = 0; j < _particleTypeCount; j++) {
                _particleTypes[i].force[j] = -_maxForce + Math.random() * _maxForce * 2.0;
                _particleTypes[i].radius[j] = _repelRadius + _maxInteractionRadius * Math.random();
            }
        }
    }
    ;
    this.initialize = function() {
        for (var i = 0; i < _particleCount; i++) {
            _particles[i] = new _particle();
            for (var j = 0; j < _maxSteps; j++) {
                _particles[i].xf[j] = 0.0;
                _particles[i].yf[j] = 0.0
            }
        }
        ;for (var i = 0; i < _particleTypeCount; i++) {
            _particleTypes[i] = new _particleType();
        }
        ;for (var i = 0; i < _particleCount; i++) {
            _particles[i].type = Math.floor((i / _particleCount) * _particleTypeCount)
        }
        ;this.initializeRandomWholeScreen();
        _particleTypes[0].color = "rgb( 200,  30,  30 )";
        _particleTypes[1].color = "rgb( 200, 150,  30 )";
        _particleTypes[2].color = "rgb( 200, 200,  30 )";
        _particleTypes[3].color = "rgb(  30, 190,  30 )";
        _particleTypes[4].color = "rgb(  70,  70, 210 )";
        _particleTypes[5].color = "rgb( 130,  40, 170 )";
        _particleTypes[6].color = "rgb( 110,  30,  30 )";
        _particleTypes[7].color = "rgb( 110,  80,  30 )";
        _particleTypes[8].color = "rgb( 120, 120,  50 )";
        _particleTypes[9].color = "rgb(  30, 100,  30 )";
        _particleTypes[10].color = "rgb(  30,  30, 160 )";
        _particleTypes[11].color = "rgb( 100,  30, 130 )";
        this.loadGenes(_defaultGenesIndex);
        this.initializeRandomWholeScreen();
        this.createButtons();
        this.renderButtons()
    }
    ;
    this.update = function() {
        for (var i = 0; i < _particleCount; i++) {
            _particles[i].xv *= (1.0 - _friction);
            _particles[i].yv *= (1.0 - _friction);
            _particles[i].x += _particles[i].xv;
            _particles[i].y += _particles[i].yv;
            this.updateParticleInteractions(i);
            for (var j = 0; j < _particleTypes[_particles[i].type].steps; j++) {
                _particles[i].xf[j] = _particles[i].xf[j + 1];
                _particles[i].yf[j] = _particles[i].yf[j + 1]
            }
            ;_particles[i].xv += _particles[i].xf[0];
            _particles[i].yv += _particles[i].yf[0];
            this.updateWallCollisions(i)
        }
        ;if (_isMouseDown) {
            for (var i = 0; i < _particleCount; i++) {
                var _deltaX = _particles[i].x - _mouseX;
                var _deltaY = _particles[i].y - _mouseY;
                var _distanceToMouse = Math.sqrt(_deltaX * _deltaX + _deltaY * _deltaY);
                if (_distanceToMouse < _mouseGrabRadius) {
                    _particles[i].xv += (_mouseX - _prevMouseX) * _mouseForce;
                    _particles[i].yv += (_mouseY - _prevMouseY) * _mouseForce
                }
            }
        }
        ;_prevMouseX = _mouseX;
        _prevMouseY = _mouseY;
        this.render();
        this.timer = setTimeout("clusters.update()", _updatePeriodMs)
    }
    ;
    this.updateParticleInteractions = function(_particleIndex) {
        var _sumXf = 0.0;
        var _sumYf = 0.0;
        var _interactionCount = 0;
        for (var i = 0; i < _particleCount; i++) {
            if (i != _particleIndex) {
                var _deltaX = _particles[i].x - _particles[_particleIndex].x;
                var _deltaY = _particles[i].y - _particles[_particleIndex].y;
                var _distanceSquared = _deltaX * _deltaX + _deltaY * _deltaY;
                var _interactionRadius = _particleTypes[_particles[_particleIndex].type].radius[_particles[i].type];
                if (_distanceSquared < _interactionRadius * _interactionRadius) {
                    var _distance = Math.sqrt(_distanceSquared);
                    var _repelDistance = _repelRadius * 2.0;
                    if (_distance < _repelDistance) {
                        if (_distance > 0.0) {
                            var _repelX = _deltaX / _distance;
                            var _repelY = _deltaY / _distance;
                            var _repelAmount = 1.0 - (_distance / _repelDistance);
                            _particles[_particleIndex].xv -= _repelX * _repelAmount;
                            _particles[_particleIndex].yv -= _repelY * _repelAmount;
                        }
                    }
                    ;var _interactionForce = _particleTypes[_particles[_particleIndex].type].force[_particles[i].type];
                    // Ilia: _distance == 0 would matter here as well? 
                    _sumXf += (_deltaX / _distance) * _interactionForce;
                    _sumYf += (_deltaY / _distance) * _interactionForce;
                    _interactionCount++
                }
            }
        }
        ;var _lastStep = _particleTypes[_particles[_particleIndex].type].steps;
        if (_interactionCount > 0) {
            _particles[_particleIndex].xf[_lastStep] = _sumXf / _interactionCount;
            _particles[_particleIndex].yf[_lastStep] = _sumYf / _interactionCount
        } else {
            _particles[_particleIndex].xf[_lastStep] = 0.0;
            _particles[_particleIndex].yf[_lastStep] = 0.0
        }
    }
    ;
    this.updateWallCollisions = function(_particleIndex) {
        if (_particles[_particleIndex].x < _particleRadius) {
            _particles[_particleIndex].x = _particleRadius;
            if (_particles[_particleIndex].xv < 0.0) {
                _particles[_particleIndex].xv *= -1.0
            }
        } else {
            if (_particles[_particleIndex].x > _width - _particleRadius) {
                _particles[_particleIndex].x = _width - _particleRadius;
                if (_particles[_particleIndex].xv > 0.0) {
                    _particles[_particleIndex].xv *= -1.0
                }
            }
        }
        ;if (_particles[_particleIndex].y < _particleRadius) {
            _particles[_particleIndex].y = _particleRadius;
            if (_particles[_particleIndex].yv < 0.0) {
                _particles[_particleIndex].yv *= -1.0
            }
        } else {
            if (_particles[_particleIndex].y > _height - _particleRadius) {
                _particles[_particleIndex].y = _height - _particleRadius;
                if (_particles[_particleIndex].yv > 0.0) {
                    _particles[_particleIndex].yv *= -1.0
                }
            }
        }
    }
    ;
    this.renderButtons = function() {
        for (var i = 0; i < _buttonCount; i++) {
            canvas.fillStyle = "rgb(  70,  80,  90 )";
            canvas.strokeStyle = "rgb( 120, 140, 160 )";
            ;canvas.fillRect(_buttons[i].x, _buttons[i].y, _buttons[i].w, _buttons[i].h);
            canvas.strokeRect(_buttons[i].x, _buttons[i].y, _buttons[i].w, _buttons[i].h);
            canvas.font = "15px sans-serif";
            canvas.textBaseline = "top";
            canvas.fillStyle = "rgb( 160, 160, 160 )";
            ;if (i == _buttonRandomizeIndex) {
                canvas.fillText("Randomize", _buttons[i].x + 5, _buttons[i].y + 5)
            } else {
                if (i == _buttonGemsIndex) {
                    canvas.fillText("Gems", _buttons[i].x + 5, _buttons[i].y + 5)
                } else {
                    if (i == _buttonAlliancesIndex) {
                        canvas.fillText("Alliances", _buttons[i].x + 5, _buttons[i].y + 5)
                    } else {
                        if (i == _buttonRedMenaceIndex) {
                            canvas.fillText("Red Menace", _buttons[i].x + 5, _buttons[i].y + 5)
                        } else {
                            if (i == _buttonAcrobatsIndex) {
                                canvas.fillText("Acrobats", _buttons[i].x + 5, _buttons[i].y + 5)
                            } else {
                                if (i == _buttonMitosisIndex) {
                                    canvas.fillText("Mitosis", _buttons[i].x + 5, _buttons[i].y + 5)
                                } else {
                                    if (i == _buttonPlanetsIndex) {
                                        canvas.fillText("Planets", _buttons[i].x + 5, _buttons[i].y + 5)
                                    } else {
                                        if (i == _buttonShipsIndex) {
                                            canvas.fillText("Ships", _buttons[i].x + 5, _buttons[i].y + 5)
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    ;
    this.render = function() {
        canvas.fillStyle = "rgba( 0, 0, 0, " + (1.0 - _backgroundTransparency) + " )";
        canvas.fillRect(0, 0, _width, _height);
        for (var i = 0; i < _particleCount; i++) {
            this.showParticle(i)
        }
        ;canvas.lineWidth = 2;
        if (_isMouseDown) {
            if (_mouseY < _height) {
                canvas.strokeStyle = "rgb( 100, 100, 100 )";
                canvas.beginPath();
                canvas.arc(_mouseX, _mouseY, _mouseGrabRadius, 0, Math.PI * 2, false);
                canvas.stroke();
                this.renderButtons()
            }
        }
        ;canvas.strokeStyle = "rgb( 100, 100, 100 )";
        canvas.strokeRect(1, 1, _width - 2, _height - 2)
    }
    ;
    this.showParticle = function(_particleIndex) {
        canvas.fillStyle = _particleTypes[_particles[_particleIndex].type].color;
        canvas.beginPath();
        canvas.arc(_particles[_particleIndex].x, _particles[_particleIndex].y, _particleRadius, 0, Math.PI * 2, false);
        canvas.fill();
        canvas.closePath()
    }
    ;
    this.mouseDown = function(_mouseEventX, _mouseEventY) {
        _isMouseDown = true;
        _mouseX = _mouseEventX;
        _mouseY = _mouseEventY;
        for (var i = 0; i < _buttonCount; i++) {
            if ((_mouseEventX > _buttons[i].x) && (_mouseEventX < _buttons[i].x + _buttons[i].w) && (_mouseEventY > _buttons[i].y) && (_mouseEventY < _buttons[i].y + _buttons[i].h)) {
                if (i == _buttonRandomizeIndex) {
                    this.bigBang()
                } else {
                    this.loadGenes(i)
                }
            }
        }
    }
    ;
    this.mouseMove = function(_mouseEventX, _mouseEventY) {
        _mouseX = _mouseEventX;
        _mouseY = _mouseEventY
    }
    ;
    this.mouseUp = function(_mouseEventX, _mouseEventY) {
        _isMouseDown = false;
        _mouseX = _mouseEventX;
        _mouseY = _mouseEventY
    }
    ;
    this.printGenes = function() {
        for (var i = 0; i < _particleTypeCount; i++) {
            for (var j = 0; j < _particleTypeCount; j++) {
                console.log("type[" + i + "].force [ " + j + "] = " + _particleTypes[i].force[j] + ";");
                console.log("type[" + i + "].radius[ " + j + "] = " + _particleTypes[i].radius[j] + ";")
            }
            ;console.log("type[" + i + "].steps = " + _particleTypes[i].steps + ";")
        }
    }
    ;
    this.loadGenes = function(_buttonIndex) {
        for (var i = 0; i < _particleTypeCount; i++) {
            for (var j = 0; j < _particleTypeCount; j++) {
                _particleTypes[i].steps = 0;
                _particleTypes[i].force[j] = -_maxForce * 0.1;
                _particleTypes[i].radius[j] = _maxInteractionRadius
            }
        }
        ;if (_buttonIndex == _buttonGemsIndex) {
            for (var i = 0; i < _particleTypeCount; i++) {
                for (var j = 0; j < _particleTypeCount; j++) {
                    _particleTypes[i].force[j] = -2.0;
                    _particleTypes[i].radius[j] = _maxInteractionRadius
                }
            }
            ;_particleTypes[1].force[1] = -3.290671030059457;
            _particleTypes[1].radius[1] = 42.49002040922642;
            _particleTypes[1].force[4] = -1.2598434370011091;
            _particleTypes[1].radius[4] = 63.842149429023266;
            _particleTypes[1].force[5] = 2.578464737161994;
            _particleTypes[1].radius[5] = 63.114551432430744;
            _particleTypes[4].force[1] = -1.1081697009503841;
            _particleTypes[4].radius[1] = 33.84286079183221;
            _particleTypes[4].force[4] = 0.526039507240057;
            _particleTypes[4].radius[4] = 18.11127431318164;
            _particleTypes[4].force[5] = 3.9443997144699097;
            _particleTypes[4].radius[5] = 48.21247752383351;
            _particleTypes[5].force[1] = 2.3572729844599962;
            _particleTypes[5].radius[1] = 76.98223288729787;
            _particleTypes[5].force[4] = -2.956161877140403;
            _particleTypes[5].radius[4] = 66.31004854664207;
            _particleTypes[5].force[5] = 2.6210244055837393;
            _particleTypes[5].radius[5] = 59.6334382481873;
            _particleTypes[0].force[0] = -3.290671030059457;
            _particleTypes[0].radius[0] = 42.49002040922642;
            _particleTypes[0].force[3] = -1.2598434370011091;
            _particleTypes[0].radius[3] = 63.842149429023266;
            _particleTypes[0].force[2] = 2.578464737161994;
            _particleTypes[0].radius[2] = 63.114551432430744;
            _particleTypes[3].force[0] = -1.1081697009503841;
            _particleTypes[3].radius[0] = 33.84286079183221;
            _particleTypes[3].force[3] = 0.526039507240057;
            _particleTypes[3].radius[3] = 18.11127431318164;
            _particleTypes[3].force[2] = 3.9443997144699097;
            _particleTypes[3].radius[2] = 48.21247752383351;
            _particleTypes[2].force[0] = 2.3572729844599962;
            _particleTypes[2].radius[0] = 76.98223288729787;
            _particleTypes[2].force[3] = -2.956161877140403;
            _particleTypes[2].radius[3] = 66.31004854664207;
            _particleTypes[2].force[2] = 2.6210244055837393;
            _particleTypes[2].radius[2] = 59.6334382481873;
            this.initializeRandomOnBottom()
        } else {
            if (_buttonIndex == _buttonAlliancesIndex) {
                for (var i = 0; i < _particleTypeCount; i++) {
                    _particleTypes[i].steps = 0;
                    for (var j = 0; j < _particleTypeCount; j++) {
                        _particleTypes[i].force[j] = -_maxForce * 0.2;
                        _particleTypes[i].radius[j] = _maxInteractionRadius
                    }
                }
                ;var _0x536bx57 = _maxForce * 0.2;
                var _0x536bx58 = _maxForce * -0.5;
                var _0x536bx59 = _maxForce * 0.5;
                for (var i = 0; i < _particleTypeCount; i++) {
                    var _0x536bx5a = i - 1;
                    var _0x536bx5b = i;
                    var _0x536bx5c = i + 1;
                    if (_0x536bx5a < 0) {
                        _0x536bx5a = _particleTypeCount - 1
                    }
                    ;if (_0x536bx5c > _particleTypeCount - 1) {
                        _0x536bx5c = 0
                    }
                    ;_particleTypes[i].force[_0x536bx5a] = _0x536bx58;
                    _particleTypes[i].force[_0x536bx5b] = _0x536bx57;
                    _particleTypes[i].force[_0x536bx5c] = _0x536bx59
                }
                ;this.initializeWithStripes()
            } else {
                if (_buttonIndex == _buttonRedMenaceIndex) {
                    for (var i = 0; i < _particleTypeCount; i++) {
                        for (var j = 0; j < _particleTypeCount; j++) {
                            _particleTypes[i].force[j] = _maxForce * 0.1;
                            _particleTypes[i].radius[j] = _maxInteractionRadius * 0.2
                        }
                    }
                    ;var _0x536bx5d = _maxForce * -1.0;
                    var _0x536bx5e = _maxForce * 1.2;
                    var _0x536bx5f = _maxInteractionRadius * 0.5;
                    var _0x536bx60 = _maxInteractionRadius * 0.9;
                    for (var i = 1; i < _particleTypeCount; i++) {
                        _particleTypes[i].force[0] = _0x536bx5d;
                        _particleTypes[i].radius[0] = _0x536bx5f * (i / _particleTypeCount);
                        _particleTypes[0].force[i] = _0x536bx5e;
                        _particleTypes[0].radius[i] = _0x536bx60
                    }
                    ;this.initializeAsDisk(20)
                } else {
                    if (_buttonIndex == _buttonAcrobatsIndex) {
                        _particleTypes[0].force[0] = -1.5508803074180229;
                        _particleTypes[0].radius[0] = 67.39870606440391;
                        _particleTypes[0].force[1] = 3.531817763582824;
                        _particleTypes[0].radius[1] = 27.965232121066215;
                        _particleTypes[0].force[2] = -2.9919035578897333;
                        _particleTypes[0].radius[2] = 10.438678512701594;
                        _particleTypes[0].force[3] = -3.8794564810173178;
                        _particleTypes[0].radius[3] = 22.65486638779329;
                        _particleTypes[0].force[4] = 2.5954984826704823;
                        _particleTypes[0].radius[4] = 60.22401393986497;
                        _particleTypes[0].force[5] = 1.3760776294881119;
                        _particleTypes[0].radius[5] = 4.845826513992259;
                        _particleTypes[0].force[6] = -2.9816940192692893;
                        _particleTypes[0].radius[6] = 29.370149467449814;
                        _particleTypes[0].force[7] = -1.45395165216312;
                        _particleTypes[0].radius[7] = 48.338890718007626;
                        _particleTypes[0].force[8] = 2.757049039564828;
                        _particleTypes[0].radius[8] = 42.58464017605024;
                        _particleTypes[0].force[9] = 3.4908970835132127;
                        _particleTypes[0].radius[9] = 64.36448967277636;
                        _particleTypes[0].force[10] = -0.24931194857345496;
                        _particleTypes[0].radius[10] = 58.32672888381189;
                        _particleTypes[0].force[11] = 1.2186156126592262;
                        _particleTypes[0].radius[11] = 71.01087918959422;
                        _particleTypes[0].steps = 0;
                        _particleTypes[1].force[0] = 3.674318534735148;
                        _particleTypes[1].radius[0] = 24.841108716472515;
                        _particleTypes[1].force[1] = 3.1508842519689413;
                        _particleTypes[1].radius[1] = 72.60388969661699;
                        _particleTypes[1].force[2] = -3.6529130461313883;
                        _particleTypes[1].radius[2] = 39.06133978399006;
                        _particleTypes[1].force[3] = 2.4141718235322926;
                        _particleTypes[1].radius[3] = 11.004824340092515;
                        _particleTypes[1].force[4] = 1.6574672176974676;
                        _particleTypes[1].radius[4] = 56.446196037025175;
                        _particleTypes[1].force[5] = -1.586790781249329;
                        _particleTypes[1].radius[5] = 7.6602382534441436;
                        _particleTypes[1].force[6] = -0.7930895186724403;
                        _particleTypes[1].radius[6] = 25.662906868391964;
                        _particleTypes[1].force[7] = -0.8069300396381909;
                        _particleTypes[1].radius[7] = 68.09663113073869;
                        _particleTypes[1].force[8] = -2.1849782173169885;
                        _particleTypes[1].radius[8] = 70.23956881737872;
                        _particleTypes[1].force[9] = 2.4217882932185617;
                        _particleTypes[1].radius[9] = 45.09886136790743;
                        _particleTypes[1].force[10] = -2.9541964866025108;
                        _particleTypes[1].radius[10] = 57.70036651488808;
                        _particleTypes[1].force[11] = 0.22365393347203444;
                        _particleTypes[1].radius[11] = 72.36404160742227;
                        _particleTypes[1].steps = 2;
                        _particleTypes[2].force[0] = 3.7041673121665433;
                        _particleTypes[2].radius[0] = 4.1210748468940475;
                        _particleTypes[2].force[1] = -0.6541684914961721;
                        _particleTypes[2].radius[1] = 33.73145094814303;
                        _particleTypes[2].force[2] = 2.3631965877408323;
                        _particleTypes[2].radius[2] = 27.16253170320861;
                        _particleTypes[2].force[3] = 0.8114009328501393;
                        _particleTypes[2].radius[3] = 46.81021502836751;
                        _particleTypes[2].force[4] = 2.202893016939555;
                        _particleTypes[2].radius[4] = 35.03742299450543;
                        _particleTypes[2].force[5] = 2.517458155421073;
                        _particleTypes[2].radius[5] = 72.12750006981254;
                        _particleTypes[2].force[6] = -2.6137628373526134;
                        _particleTypes[2].radius[6] = 29.183710600826977;
                        _particleTypes[2].force[7] = -1.944036326503463;
                        _particleTypes[2].radius[7] = 66.2336536871824;
                        _particleTypes[2].force[8] = 3.105170500843009;
                        _particleTypes[2].radius[8] = 16.541664627550116;
                        _particleTypes[2].force[9] = -2.8487102738052315;
                        _particleTypes[2].radius[9] = 31.874293994272833;
                        _particleTypes[2].force[10] = 2.405701204108299;
                        _particleTypes[2].radius[10] = 20.896340478471696;
                        _particleTypes[2].force[11] = -1.9872104514725901;
                        _particleTypes[2].radius[11] = 64.25949861656403;
                        _particleTypes[2].steps = 0;
                        _particleTypes[3].force[0] = -3.5327047147422377;
                        _particleTypes[3].radius[0] = 42.094533363233246;
                        _particleTypes[3].force[1] = 3.304651900052786;
                        _particleTypes[3].radius[1] = 53.55639630115912;
                        _particleTypes[3].force[2] = -3.346447197985178;
                        _particleTypes[3].radius[2] = 26.711798211170233;
                        _particleTypes[3].force[3] = -2.8623431373372714;
                        _particleTypes[3].radius[3] = 36.29856591118049;
                        _particleTypes[3].force[4] = 3.928523458227076;
                        _particleTypes[3].radius[4] = 53.27632019436461;
                        _particleTypes[3].force[5] = -0.7730238956740898;
                        _particleTypes[3].radius[5] = 9.333032782563164;
                        _particleTypes[3].force[6] = -1.164899553504128;
                        _particleTypes[3].radius[6] = 35.507560950604955;
                        _particleTypes[3].force[7] = -1.8784294278718292;
                        _particleTypes[3].radius[7] = 70.905918293596;
                        _particleTypes[3].force[8] = 3.6023347511440473;
                        _particleTypes[3].radius[8] = 14.81897400836538;
                        _particleTypes[3].force[9] = -0.6333877804625683;
                        _particleTypes[3].radius[9] = 34.51019072277912;
                        _particleTypes[3].force[10] = 0.2785734759295586;
                        _particleTypes[3].radius[10] = 37.89021056685358;
                        _particleTypes[3].force[11] = -1.6555789611601721;
                        _particleTypes[3].radius[11] = 18.84003380755539;
                        _particleTypes[3].steps = 2;
                        _particleTypes[4].force[0] = 1.0405184539575565;
                        _particleTypes[4].radius[0] = 7.514743589426741;
                        _particleTypes[4].force[1] = -0.8997020899746566;
                        _particleTypes[4].radius[1] = 67.36716137475668;
                        _particleTypes[4].force[2] = -2.125896285091631;
                        _particleTypes[4].radius[2] = 72.25963217988279;
                        _particleTypes[4].force[3] = -3.800964090831471;
                        _particleTypes[4].radius[3] = 47.25661248981523;
                        _particleTypes[4].force[4] = 1.3362149602899311;
                        _particleTypes[4].radius[4] = 16.54609429617129;
                        _particleTypes[4].force[5] = 1.6680261396408635;
                        _particleTypes[4].radius[5] = 20.48025116570071;
                        _particleTypes[4].force[6] = 0.6948823503622208;
                        _particleTypes[4].radius[6] = 29.180110761602176;
                        _particleTypes[4].force[7] = 3.372637853952618;
                        _particleTypes[4].radius[7] = 45.19078174988352;
                        _particleTypes[4].force[8] = 3.98273333161635;
                        _particleTypes[4].radius[8] = 64.64273400759589;
                        _particleTypes[4].force[9] = -3.3986720846285916;
                        _particleTypes[4].radius[9] = 58.06525337175576;
                        _particleTypes[4].force[10] = -3.5180538981305727;
                        _particleTypes[4].radius[10] = 51.72580625497462;
                        _particleTypes[4].force[11] = -1.8565031390695736;
                        _particleTypes[4].radius[11] = 9.365291857960674;
                        _particleTypes[4].steps = 2;
                        _particleTypes[5].force[0] = 3.370108079507993;
                        _particleTypes[5].radius[0] = 44.912137531201644;
                        _particleTypes[5].force[1] = -2.4078603815661754;
                        _particleTypes[5].radius[1] = 20.293511572542975;
                        _particleTypes[5].force[2] = -1.8085324729311711;
                        _particleTypes[5].radius[2] = 68.45202092362179;
                        _particleTypes[5].force[3] = -3.919889662494427;
                        _particleTypes[5].radius[3] = 59.604866184104864;
                        _particleTypes[5].force[4] = -0.6521550514590828;
                        _particleTypes[5].radius[4] = 23.004632444554883;
                        _particleTypes[5].force[5] = 0.558211134611474;
                        _particleTypes[5].radius[5] = 30.585164086962212;
                        _particleTypes[5].force[6] = 0.4522945985521982;
                        _particleTypes[5].radius[6] = 7.8240715455126155;
                        _particleTypes[5].force[7] = 0.30758949535140623;
                        _particleTypes[5].radius[7] = 23.939788618504192;
                        _particleTypes[5].force[8] = 3.9003099059111026;
                        _particleTypes[5].radius[8] = 44.12351099158426;
                        _particleTypes[5].force[9] = -1.9421843566768082;
                        _particleTypes[5].radius[9] = 63.40639303336535;
                        _particleTypes[5].force[10] = 2.590489586783807;
                        _particleTypes[5].radius[10] = 44.32429779199575;
                        _particleTypes[5].force[11] = 0.9932887058502438;
                        _particleTypes[5].radius[11] = 48.07948026003949;
                        _particleTypes[5].steps = 1;
                        _particleTypes[6].force[0] = 0.3684209115696735;
                        _particleTypes[6].radius[0] = 17.41238347779523;
                        _particleTypes[6].force[1] = 2.7088195480476713;
                        _particleTypes[6].radius[1] = 48.905336475075195;
                        _particleTypes[6].force[2] = 1.6813934932196197;
                        _particleTypes[6].radius[2] = 13.0610505089443;
                        _particleTypes[6].force[3] = 3.857236380649404;
                        _particleTypes[6].radius[3] = 40.359431502298634;
                        _particleTypes[6].force[4] = 2.3404834194199644;
                        _particleTypes[6].radius[4] = 25.580028201461758;
                        _particleTypes[6].force[5] = 3.788844517384625;
                        _particleTypes[6].radius[5] = 46.45202579585591;
                        _particleTypes[6].force[6] = -1.4701665965546447;
                        _particleTypes[6].radius[6] = 17.005344910462608;
                        _particleTypes[6].force[7] = 2.8645400983645306;
                        _particleTypes[6].radius[7] = 56.729154954344;
                        _particleTypes[6].force[8] = 1.0612676357485196;
                        _particleTypes[6].radius[8] = 49.80487543100134;
                        _particleTypes[6].force[9] = -1.9674120306545184;
                        _particleTypes[6].radius[9] = 49.784878954783984;
                        _particleTypes[6].force[10] = 2.8801054895983604;
                        _particleTypes[6].radius[10] = 53.70760376167216;
                        _particleTypes[6].force[11] = -2.0296815386249403;
                        _particleTypes[6].radius[11] = 20.288541608387785;
                        _particleTypes[6].steps = 1;
                        _particleTypes[7].force[0] = -0.03743633291406567;
                        _particleTypes[7].radius[0] = 32.610462682760726;
                        _particleTypes[7].force[1] = -2.684337870518025;
                        _particleTypes[7].radius[1] = 61.27478736835891;
                        _particleTypes[7].force[2] = -3.8789933763592437;
                        _particleTypes[7].radius[2] = 73.31784617206436;
                        _particleTypes[7].force[3] = -0.5631298467739345;
                        _particleTypes[7].radius[3] = 69.72303170520844;
                        _particleTypes[7].force[4] = 1.7570630565668282;
                        _particleTypes[7].radius[4] = 50.61289760433746;
                        _particleTypes[7].force[5] = -2.1798355463549033;
                        _particleTypes[7].radius[5] = 71.7637373785018;
                        _particleTypes[7].force[6] = -1.6049877040754996;
                        _particleTypes[7].radius[6] = 60.92533407295335;
                        _particleTypes[7].force[7] = -1.591301107429965;
                        _particleTypes[7].radius[7] = 69.97152912324606;
                        _particleTypes[7].force[8] = -3.3533983301685613;
                        _particleTypes[7].radius[8] = 7.13500676672375;
                        _particleTypes[7].force[9] = 1.1230119813981432;
                        _particleTypes[7].radius[9] = 44.18983340617562;
                        _particleTypes[7].force[10] = -2.307468169143;
                        _particleTypes[7].radius[10] = 60.2171290647965;
                        _particleTypes[7].force[11] = -1.8395770058122984;
                        _particleTypes[7].radius[11] = 22.550701510101792;
                        _particleTypes[7].steps = 0;
                        _particleTypes[8].force[0] = -3.8267391225226657;
                        _particleTypes[8].radius[0] = 45.68745514010258;
                        _particleTypes[8].force[1] = -1.9969967237378974;
                        _particleTypes[8].radius[1] = 5.024318159021252;
                        _particleTypes[8].force[2] = -2.934161563601082;
                        _particleTypes[8].radius[2] = 63.658697114466975;
                        _particleTypes[8].force[3] = -3.740679556129015;
                        _particleTypes[8].radius[3] = 57.49108357664291;
                        _particleTypes[8].force[4] = 2.70948489236069;
                        _particleTypes[8].radius[4] = 47.48467172819946;
                        _particleTypes[8].force[5] = -2.9200393141712517;
                        _particleTypes[8].radius[5] = 13.577739265531857;
                        _particleTypes[8].force[6] = 3.3389972867523845;
                        _particleTypes[8].radius[6] = 46.53275200437179;
                        _particleTypes[8].force[7] = 3.342232906318536;
                        _particleTypes[8].radius[7] = 57.39709950789376;
                        _particleTypes[8].force[8] = 0.1205772317923941;
                        _particleTypes[8].radius[8] = 11.221351777181964;
                        _particleTypes[8].force[9] = 2.840686537655019;
                        _particleTypes[8].radius[9] = 24.225052493176218;
                        _particleTypes[8].force[10] = -3.498083705272098;
                        _particleTypes[8].radius[10] = 57.409010924976776;
                        _particleTypes[8].force[11] = -2.8135876021806574;
                        _particleTypes[8].radius[11] = 43.8306549291934;
                        _particleTypes[8].steps = 1;
                        _particleTypes[9].force[0] = -3.520628450586944;
                        _particleTypes[9].radius[0] = 47.264275009170674;
                        _particleTypes[9].force[1] = 3.7631309894501914;
                        _particleTypes[9].radius[1] = 19.266064018762016;
                        _particleTypes[9].force[2] = -1.6837623397845327;
                        _particleTypes[9].radius[2] = 72.8702840373934;
                        _particleTypes[9].force[3] = 0.15685482622406077;
                        _particleTypes[9].radius[3] = 65.09861099562602;
                        _particleTypes[9].force[4] = -0.44688520334295845;
                        _particleTypes[9].radius[4] = 21.35613456008749;
                        _particleTypes[9].force[5] = -1.3940014924390631;
                        _particleTypes[9].radius[5] = 30.87162369561083;
                        _particleTypes[9].force[6] = -1.360858680554509;
                        _particleTypes[9].radius[6] = 34.46351338985762;
                        _particleTypes[9].force[7] = 1.266369454552068;
                        _particleTypes[9].radius[7] = 39.32344408998522;
                        _particleTypes[9].force[8] = 1.3339683465339949;
                        _particleTypes[9].radius[8] = 48.85371257026933;
                        _particleTypes[9].force[9] = 0.3952462155889229;
                        _particleTypes[9].radius[9] = 45.000388103151145;
                        _particleTypes[9].force[10] = 2.454244871367198;
                        _particleTypes[9].radius[10] = 54.771837154193655;
                        _particleTypes[9].force[11] = -1.963820282997851;
                        _particleTypes[9].radius[11] = 8.32651611832242;
                        _particleTypes[9].steps = 0;
                        _particleTypes[10].force[0] = -1.2924109492432923;
                        _particleTypes[10].radius[0] = 22.22205099169403;
                        _particleTypes[10].force[1] = -3.390344947932869;
                        _particleTypes[10].radius[1] = 5.741241174695658;
                        _particleTypes[10].force[2] = 3.469309773901921;
                        _particleTypes[10].radius[2] = 27.599270598699402;
                        _particleTypes[10].force[3] = -2.289655999656228;
                        _particleTypes[10].radius[3] = 34.16258785602287;
                        _particleTypes[10].force[4] = -3.9314782112426148;
                        _particleTypes[10].radius[4] = 36.40419966259936;
                        _particleTypes[10].force[5] = 2.9872041582373825;
                        _particleTypes[10].radius[5] = 14.095713411923002;
                        _particleTypes[10].force[6] = -1.722548529825243;
                        _particleTypes[10].radius[6] = 67.41573004208195;
                        _particleTypes[10].force[7] = 0.25393759157847917;
                        _particleTypes[10].radius[7] = 10.801089153825833;
                        _particleTypes[10].force[8] = -3.843656072293644;
                        _particleTypes[10].radius[8] = 4.365607139243316;
                        _particleTypes[10].force[9] = 0.9684566345251966;
                        _particleTypes[10].radius[9] = 10.070848941872791;
                        _particleTypes[10].force[10] = 3.2677922850675305;
                        _particleTypes[10].radius[10] = 25.41264692390017;
                        _particleTypes[10].force[11] = 1.7862351778093828;
                        _particleTypes[10].radius[11] = 51.83793074487167;
                        _particleTypes[10].steps = 3;
                        _particleTypes[11].force[0] = 1.0133954345825291;
                        _particleTypes[11].radius[0] = 12.196461417685393;
                        _particleTypes[11].force[1] = 0.7041322369226215;
                        _particleTypes[11].radius[1] = 47.53519654556223;
                        _particleTypes[11].force[2] = 0.8393465902134665;
                        _particleTypes[11].radius[2] = 24.832950998589283;
                        _particleTypes[11].force[3] = -0.5282457853831666;
                        _particleTypes[11].radius[3] = 7.80246783597418;
                        _particleTypes[11].force[4] = 3.839173725054861;
                        _particleTypes[11].radius[4] = 64.13406527360796;
                        _particleTypes[11].force[5] = 1.059743178817783;
                        _particleTypes[11].radius[5] = 14.896091762401866;
                        _particleTypes[11].force[6] = 2.1497757098979644;
                        _particleTypes[11].radius[6] = 58.76028779722146;
                        _particleTypes[11].force[7] = 3.68522924244772;
                        _particleTypes[11].radius[7] = 29.997400720396534;
                        _particleTypes[11].force[8] = 0.45826601561131675;
                        _particleTypes[11].radius[8] = 32.55112761551895;
                        _particleTypes[11].force[9] = 1.2231339691237775;
                        _particleTypes[11].radius[9] = 5.779111349260692;
                        _particleTypes[11].force[10] = 3.219695583879032;
                        _particleTypes[11].radius[10] = 70.0508959821433;
                        _particleTypes[11].force[11] = 2.3973160498915895;
                        _particleTypes[11].radius[11] = 46.74756654973118;
                        _particleTypes[11].steps = 1;
                        this.initializeRandomWholeScreen()
                    } else {
                        if (_buttonIndex == _buttonMitosisIndex) {
                            for (var i = 2; i < _particleTypeCount; i++) {
                                for (var j = 0; j < _particleTypeCount; j++) {
                                    _particleTypes[i].force[j] = j / _particleTypeCount * 2.0 - i / _particleTypeCount * 0.7;
                                    _particleTypes[i].radius[j] = _maxInteractionRadius * 0.5
                                }
                            }
                            ;this.initializeAsDisk(30)
                        } else {
                            if (_buttonIndex == _buttonPlanetsIndex) {
                                for (var i = 0; i < _particleTypeCount; i++) {
                                    for (var j = 0; j < _particleTypeCount; j++) {
                                        _particleTypes[i].force[j] = -1.0;
                                        _particleTypes[i].radius[j] = _maxInteractionRadius * 0.5
                                    }
                                }
                                ;var _0x536bx42 = 1.0;
                                var _0x536bx61 = 50.0;
                                var _0x536bx43 = 0;
                                var _0x536bx62 = 0;
                                _0x536bx43 = 0;
                                _0x536bx62 = 6;
                                this.setPlanetValues(_0x536bx43, _0x536bx62, _0x536bx42, _0x536bx61);
                                this.setPlanetValues(_0x536bx62, _0x536bx43, _0x536bx42, _0x536bx61);
                                _0x536bx43 = 0;
                                _0x536bx62 = 7;
                                this.setPlanetValues(_0x536bx43, _0x536bx62, _0x536bx42, _0x536bx61);
                                this.setPlanetValues(_0x536bx62, _0x536bx43, _0x536bx42, _0x536bx61);
                                _0x536bx43 = 0;
                                _0x536bx62 = 1;
                                this.setPlanetValues(_0x536bx43, _0x536bx62, _0x536bx42, _0x536bx61);
                                this.setPlanetValues(_0x536bx62, _0x536bx43, _0x536bx42, _0x536bx61);
                                _0x536bx43 = 1;
                                _0x536bx62 = 6;
                                this.setPlanetValues(_0x536bx43, _0x536bx62, _0x536bx42, _0x536bx61);
                                this.setPlanetValues(_0x536bx62, _0x536bx43, _0x536bx42, _0x536bx61);
                                _0x536bx43 = 1;
                                _0x536bx62 = 7;
                                this.setPlanetValues(_0x536bx43, _0x536bx62, _0x536bx42, _0x536bx61);
                                this.setPlanetValues(_0x536bx62, _0x536bx43, _0x536bx42, _0x536bx61);
                                _0x536bx43 = 6;
                                _0x536bx62 = 7;
                                this.setPlanetValues(_0x536bx43, _0x536bx62, _0x536bx42, _0x536bx61);
                                this.setPlanetValues(_0x536bx62, _0x536bx43, _0x536bx42, _0x536bx61);
                                _0x536bx43 = 2;
                                _0x536bx62 = 8;
                                this.setPlanetValues(_0x536bx43, _0x536bx62, _0x536bx42, _0x536bx61);
                                this.setPlanetValues(_0x536bx62, _0x536bx43, _0x536bx42, _0x536bx61);
                                _0x536bx43 = 2;
                                _0x536bx62 = 9;
                                this.setPlanetValues(_0x536bx43, _0x536bx62, _0x536bx42, _0x536bx61);
                                this.setPlanetValues(_0x536bx62, _0x536bx43, _0x536bx42, _0x536bx61);
                                _0x536bx43 = 2;
                                _0x536bx62 = 3;
                                this.setPlanetValues(_0x536bx43, _0x536bx62, _0x536bx42, _0x536bx61);
                                this.setPlanetValues(_0x536bx62, _0x536bx43, _0x536bx42, _0x536bx61);
                                _0x536bx43 = 3;
                                _0x536bx62 = 8;
                                this.setPlanetValues(_0x536bx43, _0x536bx62, _0x536bx42, _0x536bx61);
                                this.setPlanetValues(_0x536bx62, _0x536bx43, _0x536bx42, _0x536bx61);
                                _0x536bx43 = 3;
                                _0x536bx62 = 9;
                                this.setPlanetValues(_0x536bx43, _0x536bx62, _0x536bx42, _0x536bx61);
                                this.setPlanetValues(_0x536bx62, _0x536bx43, _0x536bx42, _0x536bx61);
                                _0x536bx43 = 8;
                                _0x536bx62 = 9;
                                this.setPlanetValues(_0x536bx43, _0x536bx62, _0x536bx42, _0x536bx61);
                                this.setPlanetValues(_0x536bx62, _0x536bx43, _0x536bx42, _0x536bx61);
                                _0x536bx43 = 4;
                                _0x536bx62 = 10;
                                this.setPlanetValues(_0x536bx43, _0x536bx62, _0x536bx42, _0x536bx61);
                                this.setPlanetValues(_0x536bx62, _0x536bx43, _0x536bx42, _0x536bx61);
                                _0x536bx43 = 4;
                                _0x536bx62 = 11;
                                this.setPlanetValues(_0x536bx43, _0x536bx62, _0x536bx42, _0x536bx61);
                                this.setPlanetValues(_0x536bx62, _0x536bx43, _0x536bx42, _0x536bx61);
                                _0x536bx43 = 4;
                                _0x536bx62 = 5;
                                this.setPlanetValues(_0x536bx43, _0x536bx62, _0x536bx42, _0x536bx61);
                                this.setPlanetValues(_0x536bx62, _0x536bx43, _0x536bx42, _0x536bx61);
                                _0x536bx43 = 5;
                                _0x536bx62 = 10;
                                this.setPlanetValues(_0x536bx43, _0x536bx62, _0x536bx42, _0x536bx61);
                                this.setPlanetValues(_0x536bx62, _0x536bx43, _0x536bx42, _0x536bx61);
                                _0x536bx43 = 5;
                                _0x536bx62 = 11;
                                this.setPlanetValues(_0x536bx43, _0x536bx62, _0x536bx42, _0x536bx61);
                                this.setPlanetValues(_0x536bx62, _0x536bx43, _0x536bx42, _0x536bx61);
                                _0x536bx43 = 10;
                                _0x536bx62 = 11;
                                this.setPlanetValues(_0x536bx43, _0x536bx62, _0x536bx42, _0x536bx61);
                                this.setPlanetValues(_0x536bx62, _0x536bx43, _0x536bx42, _0x536bx61);
                                this.initializeAsDisk(80)
                            } else {
                                if (_buttonIndex == _buttonShipsIndex) {
                                    var _0x536bx63 = -1.0;
                                    var _0x536bx64 = 10.0;
                                    _particleTypes[0].force[0] = _0x536bx63;
                                    _particleTypes[0].radius[0] = _0x536bx64;
                                    _particleTypes[0].force[1] = _0x536bx63;
                                    _particleTypes[0].radius[1] = _0x536bx64;
                                    _particleTypes[0].force[2] = _0x536bx63;
                                    _particleTypes[0].radius[2] = _0x536bx64;
                                    _particleTypes[0].force[3] = _0x536bx63;
                                    _particleTypes[0].radius[3] = _0x536bx64;
                                    _particleTypes[0].force[4] = _0x536bx63;
                                    _particleTypes[0].radius[4] = _0x536bx64;
                                    _particleTypes[0].force[5] = _0x536bx63;
                                    _particleTypes[0].radius[5] = _0x536bx64;
                                    _particleTypes[1].force[0] = 0.0;
                                    _particleTypes[1].radius[0] = 41.87992572411895;
                                    _particleTypes[1].force[1] = 0.6188263148069382;
                                    _particleTypes[1].radius[1] = 31.57806908339262;
                                    _particleTypes[1].force[2] = -2.252236846834421;
                                    _particleTypes[1].radius[2] = 16.67971832305193;
                                    _particleTypes[1].force[3] = 2.9319324381649494;
                                    _particleTypes[1].radius[3] = 75.86216926202178;
                                    _particleTypes[1].force[4] = 3.160645740106702;
                                    _particleTypes[1].radius[4] = 28.880391377955675;
                                    _particleTypes[1].force[5] = 1.0297179147601128;
                                    _particleTypes[1].radius[5] = 59.19801760092378;
                                    _particleTypes[2].force[0] = 0.0;
                                    _particleTypes[2].radius[0] = 49.67192802205682;
                                    _particleTypes[2].force[1] = -3.264488408342004;
                                    _particleTypes[2].radius[1] = 8.111502636224031;
                                    _particleTypes[2].force[2] = 3.478301437571645;
                                    _particleTypes[2].radius[2] = 81.76046648621559;
                                    _particleTypes[2].force[3] = -3.4177507925778627;
                                    _particleTypes[2].radius[3] = 48.5528220012784;
                                    _particleTypes[2].force[4] = -3.999166540801525;
                                    _particleTypes[2].radius[4] = 16.489134017378092;
                                    _particleTypes[2].force[5] = 0.6649601068347692;
                                    _particleTypes[2].radius[5] = 37.668375723063946;
                                    _particleTypes[3].force[0] = 0.0;
                                    _particleTypes[3].radius[0] = 21.195324823260307;
                                    _particleTypes[3].force[1] = 1.8835953641682863;
                                    _particleTypes[3].radius[1] = 41.92278680950403;
                                    _particleTypes[3].force[2] = 3.05437408387661;
                                    _particleTypes[3].radius[2] = 71.93124115094543;
                                    _particleTypes[3].force[3] = 0.30829014256596565;
                                    _particleTypes[3].radius[3] = 29.373187363147736;
                                    _particleTypes[3].force[4] = 2.692530371248722;
                                    _particleTypes[3].radius[4] = 17.34831178188324;
                                    _particleTypes[3].force[5] = -3.504735803231597;
                                    _particleTypes[3].radius[5] = 35.28821248188615;
                                    _particleTypes[4].force[0] = 0.0;
                                    _particleTypes[4].radius[0] = 35.6813519410789;
                                    _particleTypes[4].force[1] = -2.2478953283280134;
                                    _particleTypes[4].radius[1] = 29.27869377285242;
                                    _particleTypes[4].force[2] = 1.5714976619929075;
                                    _particleTypes[4].radius[2] = 67.66308366879821;
                                    _particleTypes[4].force[3] = 1.4469843301922083;
                                    _particleTypes[4].radius[3] = 24.738862734287977;
                                    _particleTypes[4].force[4] = -3.206526968628168;
                                    _particleTypes[4].radius[4] = 8.246950801461935;
                                    _particleTypes[4].force[5] = -3.382426990196109;
                                    _particleTypes[4].radius[5] = 20.83147009462118;
                                    _particleTypes[5].force[0] = 0.0;
                                    _particleTypes[5].radius[0] = 58.359155502170324;
                                    _particleTypes[5].force[1] = 0.5229634866118431;
                                    _particleTypes[5].radius[1] = 22.19472612813115;
                                    _particleTypes[5].force[2] = -0.3390012998133898;
                                    _particleTypes[5].radius[2] = 59.756876077502966;
                                    _particleTypes[5].force[3] = 0.20365052670240402;
                                    _particleTypes[5].radius[3] = 29.851365625858307;
                                    _particleTypes[5].force[4] = 2.2390960846096277;
                                    _particleTypes[5].radius[4] = 67.69483275339007;
                                    _particleTypes[5].force[5] = 1.7939001806080341;
                                    _particleTypes[5].radius[5] = 25.740952897816896;
                                    this.initializeAsIsolatedBlobs()
                                } else {
                                    if (_buttonIndex == _defaultGenesIndex) {
                                        _particleTypes[0].force[0] = 1.301252300661762;
                                        _particleTypes[0].radius[0] = 68.91413348891803;
                                        _particleTypes[0].force[1] = -2.230152907846133;
                                        _particleTypes[0].radius[1] = 19.281494768946192;
                                        _particleTypes[0].force[2] = 0.025212746508003647;
                                        _particleTypes[0].radius[2] = 50.20624742108828;
                                        _particleTypes[0].force[3] = -3.600271341394796;
                                        _particleTypes[0].radius[3] = 34.85721850355756;
                                        _particleTypes[0].force[4] = -1.061633091334187;
                                        _particleTypes[0].radius[4] = 14.04677509180996;
                                        _particleTypes[0].force[5] = 2.2695255146242133;
                                        _particleTypes[0].radius[5] = 14.637383485166543;
                                        _particleTypes[0].force[6] = -0.5998725570640531;
                                        _particleTypes[0].radius[6] = 39.7986966119487;
                                        _particleTypes[0].force[7] = 2.7600834166481185;
                                        _particleTypes[0].radius[7] = 57.476982699257505;
                                        _particleTypes[0].force[8] = 3.9165354481056998;
                                        _particleTypes[0].radius[8] = 45.9771871484167;
                                        _particleTypes[0].force[9] = -3.731315996097976;
                                        _particleTypes[0].radius[9] = 8.560944028117882;
                                        _particleTypes[0].force[10] = 2.074666274923258;
                                        _particleTypes[0].radius[10] = 24.232277191010564;
                                        _particleTypes[0].force[11] = 1.0173633142136023;
                                        _particleTypes[0].radius[11] = 48.25235879938472;
                                        _particleTypes[0].steps = 0;
                                        _particleTypes[1].force[0] = -0.878552550489685;
                                        _particleTypes[1].radius[0] = 73.1395658125685;
                                        _particleTypes[1].force[1] = 3.4426561966786657;
                                        _particleTypes[1].radius[1] = 53.44378619860082;
                                        _particleTypes[1].force[2] = 1.8347969868641716;
                                        _particleTypes[1].radius[2] = 9.923088760356242;
                                        _particleTypes[1].force[3] = -2.869507890058424;
                                        _particleTypes[1].radius[3] = 71.45572896784721;
                                        _particleTypes[1].force[4] = -3.4834539888722613;
                                        _particleTypes[1].radius[4] = 54.691385476793464;
                                        _particleTypes[1].force[5] = 3.3162109977108116;
                                        _particleTypes[1].radius[5] = 59.767429038243705;
                                        _particleTypes[1].force[6] = -3.3464992002107916;
                                        _particleTypes[1].radius[6] = 33.11372959126176;
                                        _particleTypes[1].force[7] = -0.3215693389859542;
                                        _particleTypes[1].radius[7] = 10.038893532909533;
                                        _particleTypes[1].force[8] = -0.41840903396395746;
                                        _particleTypes[1].radius[8] = 21.7770068521151;
                                        _particleTypes[1].force[9] = -2.039441505305959;
                                        _particleTypes[1].radius[9] = 66.29471233318375;
                                        _particleTypes[1].force[10] = -3.953137560526528;
                                        _particleTypes[1].radius[10] = 63.99666358353694;
                                        _particleTypes[1].force[11] = -0.8487458409824082;
                                        _particleTypes[1].radius[11] = 44.142402696874775;
                                        _particleTypes[1].steps = 3;
                                        _particleTypes[2].force[0] = 2.7908707226164324;
                                        _particleTypes[2].radius[0] = 15.006860119121885;
                                        _particleTypes[2].force[1] = -2.302292926531038;
                                        _particleTypes[2].radius[1] = 30.24471514095326;
                                        _particleTypes[2].force[2] = 0.3003748691634218;
                                        _particleTypes[2].radius[2] = 15.565204617111014;
                                        _particleTypes[2].force[3] = 1.8138447445137942;
                                        _particleTypes[2].radius[3] = 11.406602732102792;
                                        _particleTypes[2].force[4] = 2.791729664710756;
                                        _particleTypes[2].radius[4] = 14.30944425512956;
                                        _particleTypes[2].force[5] = -3.6070134541564745;
                                        _particleTypes[2].radius[5] = 30.037839738124323;
                                        _particleTypes[2].force[6] = -2.266641735094261;
                                        _particleTypes[2].radius[6] = 41.81347096013967;
                                        _particleTypes[2].force[7] = -0.6991280942869658;
                                        _particleTypes[2].radius[7] = 15.293417440430732;
                                        _particleTypes[2].force[8] = -0.3042128156770456;
                                        _particleTypes[2].radius[8] = 6.656558170421874;
                                        _particleTypes[2].force[9] = -3.3558678366719903;
                                        _particleTypes[2].radius[9] = 73.04306350850467;
                                        _particleTypes[2].force[10] = -3.307835757445231;
                                        _particleTypes[2].radius[10] = 24.125867234594402;
                                        _particleTypes[2].force[11] = 1.8482586070142322;
                                        _particleTypes[2].radius[11] = 12.446222888936877;
                                        _particleTypes[2].steps = 1;
                                        _particleTypes[3].force[0] = -1.8891097143695923;
                                        _particleTypes[3].radius[0] = 31.83274218524956;
                                        _particleTypes[3].force[1] = 3.261176823902879;
                                        _particleTypes[3].radius[1] = 58.82597659710637;
                                        _particleTypes[3].force[2] = 0.8057206751389643;
                                        _particleTypes[3].radius[2] = 10.33016114551994;
                                        _particleTypes[3].force[3] = -0.8682907675914784;
                                        _particleTypes[3].radius[3] = 71.22823268411284;
                                        _particleTypes[3].force[4] = 1.7173051510730062;
                                        _particleTypes[3].radius[4] = 56.24430448075387;
                                        _particleTypes[3].force[5] = 3.4909563757143296;
                                        _particleTypes[3].radius[5] = 59.449049389308776;
                                        _particleTypes[3].force[6] = -2.0293306233665973;
                                        _particleTypes[3].radius[6] = 14.812189618807356;
                                        _particleTypes[3].force[7] = -1.0078304146888453;
                                        _particleTypes[3].radius[7] = 66.02389292925285;
                                        _particleTypes[3].force[8] = -3.7531740814148886;
                                        _particleTypes[3].radius[8] = 50.16542510939933;
                                        _particleTypes[3].force[9] = 2.445249775696258;
                                        _particleTypes[3].radius[9] = 20.436319859519344;
                                        _particleTypes[3].force[10] = -3.343349605076938;
                                        _particleTypes[3].radius[10] = 57.980543972121986;
                                        _particleTypes[3].force[11] = -0.6482264100225787;
                                        _particleTypes[3].radius[11] = 12.595172221111563;
                                        _particleTypes[3].steps = 1;
                                        _particleTypes[4].force[0] = 1.5247906685993087;
                                        _particleTypes[4].radius[0] = 50.84299555372993;
                                        _particleTypes[4].force[1] = 3.8324193942817217;
                                        _particleTypes[4].radius[1] = 21.215726331693567;
                                        _particleTypes[4].force[2] = 2.0960926860649227;
                                        _particleTypes[4].radius[2] = 34.96692963551149;
                                        _particleTypes[4].force[3] = 0.9891089739076921;
                                        _particleTypes[4].radius[3] = 17.047756420354553;
                                        _particleTypes[4].force[4] = -3.9807374985053947;
                                        _particleTypes[4].radius[4] = 27.66793840739101;
                                        _particleTypes[4].force[5] = -0.534576263593955;
                                        _particleTypes[4].radius[5] = 50.330916124957135;
                                        _particleTypes[4].force[6] = 2.0132157003199325;
                                        _particleTypes[4].radius[6] = 68.12098983638967;
                                        _particleTypes[4].force[7] = -3.689865926907501;
                                        _particleTypes[4].radius[7] = 50.87508777579631;
                                        _particleTypes[4].force[8] = -0.8523962616990737;
                                        _particleTypes[4].radius[8] = 27.3777298130807;
                                        _particleTypes[4].force[9] = 1.816515708536647;
                                        _particleTypes[4].radius[9] = 65.54701826373172;
                                        _particleTypes[4].force[10] = 0.03632035506255882;
                                        _particleTypes[4].radius[10] = 46.24378529911964;
                                        _particleTypes[4].force[11] = 3.7143076904334382;
                                        _particleTypes[4].radius[11] = 19.206151550814663;
                                        _particleTypes[4].steps = 2;
                                        _particleTypes[5].force[0] = 3.7417528729970826;
                                        _particleTypes[5].radius[0] = 18.085066621465696;
                                        _particleTypes[5].force[1] = -3.882267125691259;
                                        _particleTypes[5].radius[1] = 65.19133973357555;
                                        _particleTypes[5].force[2] = -1.3366205540443268;
                                        _particleTypes[5].radius[2] = 40.887140021918945;
                                        _particleTypes[5].force[3] = -1.414634828987328;
                                        _particleTypes[5].radius[3] = 15.693310048063701;
                                        _particleTypes[5].force[4] = -3.7340456640909157;
                                        _particleTypes[5].radius[4] = 39.1724475973607;
                                        _particleTypes[5].force[5] = -2.1228845472275815;
                                        _particleTypes[5].radius[5] = 29.28317398734494;
                                        _particleTypes[5].force[6] = 1.4171645108196174;
                                        _particleTypes[5].radius[6] = 43.547155378705995;
                                        _particleTypes[5].force[7] = 1.4786718768546994;
                                        _particleTypes[5].radius[7] = 14.257070666883868;
                                        _particleTypes[5].force[8] = 2.2236719538502125;
                                        _particleTypes[5].radius[8] = 21.715934427479596;
                                        _particleTypes[5].force[9] = 3.2273831963086295;
                                        _particleTypes[5].radius[9] = 33.662117235940826;
                                        _particleTypes[5].force[10] = -2.1497283179778197;
                                        _particleTypes[5].radius[10] = 55.50955865972704;
                                        _particleTypes[5].force[11] = -2.176975451442974;
                                        _particleTypes[5].radius[11] = 5.03230716568692;
                                        _particleTypes[5].steps = 1;
                                        _particleTypes[6].force[0] = -1.4962612968150513;
                                        _particleTypes[6].radius[0] = 67.01409119109898;
                                        _particleTypes[6].force[1] = 0.8314027602409304;
                                        _particleTypes[6].radius[1] = 12.619103089469384;
                                        _particleTypes[6].force[2] = -3.700335013635998;
                                        _particleTypes[6].radius[2] = 48.198346564538454;
                                        _particleTypes[6].force[3] = 3.0494201358091235;
                                        _particleTypes[6].radius[3] = 14.968120263872137;
                                        _particleTypes[6].force[4] = 0.8469212598098306;
                                        _particleTypes[6].radius[4] = 70.47744659254101;
                                        _particleTypes[6].force[5] = 1.8460392025046346;
                                        _particleTypes[6].radius[5] = 72.76057432407893;
                                        _particleTypes[6].force[6] = -2.9496249058035655;
                                        _particleTypes[6].radius[6] = 69.67476166673089;
                                        _particleTypes[6].force[7] = 1.166603371002779;
                                        _particleTypes[6].radius[7] = 54.982843224775785;
                                        _particleTypes[6].force[8] = -1.0737530820036163;
                                        _particleTypes[6].radius[8] = 27.813970762827335;
                                        _particleTypes[6].force[9] = 2.493034446581616;
                                        _particleTypes[6].radius[9] = 25.280644989562386;
                                        _particleTypes[6].force[10] = -2.7361943079695497;
                                        _particleTypes[6].radius[10] = 4.806918605555558;
                                        _particleTypes[6].force[11] = -0.47338255101352367;
                                        _particleTypes[6].radius[11] = 39.03865562678257;
                                        _particleTypes[6].steps = 3;
                                        _particleTypes[7].force[0] = -3.6097174443282167;
                                        _particleTypes[7].radius[0] = 15.515706747676012;
                                        _particleTypes[7].force[1] = -2.079151526042054;
                                        _particleTypes[7].radius[1] = 53.823367578827224;
                                        _particleTypes[7].force[2] = -0.9688800146765129;
                                        _particleTypes[7].radius[2] = 71.04928842085236;
                                        _particleTypes[7].force[3] = 1.9432877464855718;
                                        _particleTypes[7].radius[3] = 50.130661700389616;
                                        _particleTypes[7].force[4] = -3.9596580575773057;
                                        _particleTypes[7].radius[4] = 10.504576799202908;
                                        _particleTypes[7].force[5] = 3.446095283963553;
                                        _particleTypes[7].radius[5] = 39.86429439275851;
                                        _particleTypes[7].force[6] = 0.7215452088733869;
                                        _particleTypes[7].radius[6] = 10.845386814135477;
                                        _particleTypes[7].force[7] = 1.34024507566226;
                                        _particleTypes[7].radius[7] = 6.521026205380776;
                                        _particleTypes[7].force[8] = 3.167896724392387;
                                        _particleTypes[7].radius[8] = 10.675387382172172;
                                        _particleTypes[7].force[9] = -3.7149247395354497;
                                        _particleTypes[7].radius[9] = 52.52842748558024;
                                        _particleTypes[7].force[10] = 1.337207780616108;
                                        _particleTypes[7].radius[10] = 21.47134090835949;
                                        _particleTypes[7].force[11] = -3.1284337488453247;
                                        _particleTypes[7].radius[11] = 53.83813581818488;
                                        _particleTypes[7].steps = 2;
                                        _particleTypes[8].force[0] = -3.49877566761044;
                                        _particleTypes[8].radius[0] = 22.3844721653881;
                                        _particleTypes[8].force[1] = -0.7380530354176944;
                                        _particleTypes[8].radius[1] = 22.89905177986136;
                                        _particleTypes[8].force[2] = -3.7032591582510612;
                                        _particleTypes[8].radius[2] = 6.613545419732471;
                                        _particleTypes[8].force[3] = 1.8863546832098343;
                                        _particleTypes[8].radius[3] = 17.97034326003615;
                                        _particleTypes[8].force[4] = -1.4139065702788365;
                                        _particleTypes[8].radius[4] = 65.35657881883068;
                                        _particleTypes[8].force[5] = 3.877140837617972;
                                        _particleTypes[8].radius[5] = 68.86019146529489;
                                        _particleTypes[8].force[6] = -3.573767884241663;
                                        _particleTypes[8].radius[6] = 48.437748318180226;
                                        _particleTypes[8].force[7] = 3.2007688858021;
                                        _particleTypes[8].radius[7] = 24.64146314940889;
                                        _particleTypes[8].force[8] = -0.8355112432959739;
                                        _particleTypes[8].radius[8] = 62.79873488472607;
                                        _particleTypes[8].force[9] = 3.034799946358582;
                                        _particleTypes[8].radius[9] = 16.75794989832559;
                                        _particleTypes[8].force[10] = -1.7382049201149794;
                                        _particleTypes[8].radius[10] = 10.57434402653907;
                                        _particleTypes[8].force[11] = 1.1486423971288886;
                                        _particleTypes[8].radius[11] = 49.824041804832476;
                                        _particleTypes[8].steps = 1;
                                        _particleTypes[9].force[0] = 1.0731478206883196;
                                        _particleTypes[9].radius[0] = 68.72895103833746;
                                        _particleTypes[9].force[1] = 0.26222636066114546;
                                        _particleTypes[9].radius[1] = 19.069343142340895;
                                        _particleTypes[9].force[2] = 0.3887251224968171;
                                        _particleTypes[9].radius[2] = 73.3672224548934;
                                        _particleTypes[9].force[3] = 3.4141507473220596;
                                        _particleTypes[9].radius[3] = 9.725116253049109;
                                        _particleTypes[9].force[4] = -3.884097223556296;
                                        _particleTypes[9].radius[4] = 37.642440612263044;
                                        _particleTypes[9].force[5] = 3.1487584745735138;
                                        _particleTypes[9].radius[5] = 16.80003162878174;
                                        _particleTypes[9].force[6] = 0.6106993245984516;
                                        _particleTypes[9].radius[6] = 61.187397942396814;
                                        _particleTypes[9].force[7] = 2.792852498337794;
                                        _particleTypes[9].radius[7] = 5.793493766075878;
                                        _particleTypes[9].force[8] = 0.126081191868046;
                                        _particleTypes[9].radius[8] = 67.34340521187343;
                                        _particleTypes[9].force[9] = -2.551352233904712;
                                        _particleTypes[9].radius[9] = 63.90960946733686;
                                        _particleTypes[9].force[10] = -2.4499659573121626;
                                        _particleTypes[9].radius[10] = 6.06151277816798;
                                        _particleTypes[9].force[11] = 2.578560806664809;
                                        _particleTypes[9].radius[11] = 26.81689264117286;
                                        _particleTypes[9].steps = 1;
                                        _particleTypes[10].force[0] = 0.5438641693915862;
                                        _particleTypes[10].radius[0] = 40.647161374893386;
                                        _particleTypes[10].force[1] = -1.5572945268240233;
                                        _particleTypes[10].radius[1] = 4.003731264303133;
                                        _particleTypes[10].force[2] = -0.9295880157507632;
                                        _particleTypes[10].radius[2] = 9.290083947389116;
                                        _particleTypes[10].force[3] = 3.3684327168682255;
                                        _particleTypes[10].radius[3] = 56.97394542026792;
                                        _particleTypes[10].force[4] = 3.375160315076899;
                                        _particleTypes[10].radius[4] = 67.52586543345585;
                                        _particleTypes[10].force[5] = 2.3755512897504563;
                                        _particleTypes[10].radius[5] = 53.33423717366148;
                                        _particleTypes[10].force[6] = -3.0800779009693358;
                                        _particleTypes[10].radius[6] = 31.38705343304113;
                                        _particleTypes[10].force[7] = -0.8826845940951689;
                                        _particleTypes[10].radius[7] = 44.10305389149001;
                                        _particleTypes[10].force[8] = 0.5903061653438062;
                                        _particleTypes[10].radius[8] = 47.65573962231717;
                                        _particleTypes[10].force[9] = -1.495346297502568;
                                        _particleTypes[10].radius[9] = 28.143102381592566;
                                        _particleTypes[10].force[10] = 1.2944811157500613;
                                        _particleTypes[10].radius[10] = 30.14403497049999;
                                        _particleTypes[10].force[11] = 1.6273288764500418;
                                        _particleTypes[10].radius[11] = 5.624491160462101;
                                        _particleTypes[10].steps = 3;
                                        _particleTypes[11].force[0] = -3.6908515823735932;
                                        _particleTypes[11].radius[0] = 19.958674522537557;
                                        _particleTypes[11].force[1] = -2.9414271483860333;
                                        _particleTypes[11].radius[1] = 8.768831395133722;
                                        _particleTypes[11].force[2] = 0.7358203740090055;
                                        _particleTypes[11].radius[2] = 46.17011082255885;
                                        _particleTypes[11].force[3] = -1.0382200711699987;
                                        _particleTypes[11].radius[3] = 23.830468222492748;
                                        _particleTypes[11].force[4] = -3.461169836080698;
                                        _particleTypes[11].radius[4] = 33.74219201877762;
                                        _particleTypes[11].force[5] = 0.6023675325651059;
                                        _particleTypes[11].radius[5] = 28.39065406028318;
                                        _particleTypes[11].force[6] = -0.3085846283162841;
                                        _particleTypes[11].radius[6] = 42.29495147131925;
                                        _particleTypes[11].force[7] = -2.0352051651272447;
                                        _particleTypes[11].radius[7] = 65.96143732109421;
                                        _particleTypes[11].force[8] = -2.5940385028695108;
                                        _particleTypes[11].radius[8] = 61.0815637031235;
                                        _particleTypes[11].force[9] = 3.9320609739073262;
                                        _particleTypes[11].radius[9] = 29.57446030145812;
                                        _particleTypes[11].force[10] = -0.3984974136625308;
                                        _particleTypes[11].radius[10] = 58.427003216269476;
                                        _particleTypes[11].force[11] = 1.163025105974958;
                                        _particleTypes[11].radius[11] = 47.192640992350405;
                                        _particleTypes[11].steps = 0
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    ;
    this.setPlanetValues = function(_0x536bx43, _0x536bx62, _0x536bx42, _0x536bx61) {
        _particleTypes[_0x536bx43].force[_0x536bx43] = _0x536bx42;
        _particleTypes[_0x536bx43].radius[_0x536bx43] = _0x536bx61;
        _particleTypes[_0x536bx43].force[_0x536bx62] = _0x536bx42;
        _particleTypes[_0x536bx43].radius[_0x536bx62] = _0x536bx61;
        _particleTypes[_0x536bx62].force[_0x536bx43] = _0x536bx42;
        _particleTypes[_0x536bx62].radius[_0x536bx43] = _0x536bx61;
        _particleTypes[_0x536bx62].force[_0x536bx62] = _0x536bx42;
        _particleTypes[_0x536bx62].radius[_0x536bx62] = _0x536bx61
    }
    ;
    this.bigBang = function() {
        this.initializeAsDisk(10);
        this.randomizeGenes()
    }
    ;
    this.initializeRandomWholeScreen = function() {
        for (var i = 0; i < _particleCount; i++) {
            _particles[i].x = Math.random() * _width;
            _particles[i].y = Math.random() * _height
        }
        ;this.clearVelocities()
    }
    ;
    this.initializeAsDisk = function(_maxDistanceFromCenter) {
        for (var i = 0; i < _particleCount; i++) {
            var _randomValue = Math.random();
            var _distanceFromCenter = Math.sqrt(Math.random()) * _maxDistanceFromCenter;
            var _angleFromCenter = _randomValue * Math.PI * 2.0;
            _particles[i].x = _width / 2.0 + _distanceFromCenter * Math.sin(_angleFromCenter);
            _particles[i].y = _height / 2.0 + _distanceFromCenter * Math.cos(_angleFromCenter)
        }
        ;this.clearVelocities()
    }
    ;
    this.initializeAsIsolatedBlobs = function() {
        var _0x536bx68 = 210;
        var _0x536bx69 = 12;
        for (var i = 0; i < _particleTypeCount; i++) {
            var _0x536bx44 = 0;
            var _0x536bx55 = _height / 2.0;
            for (var j = 0; j < _particleCount; j++) {
                if (_particles[j].type == i) {
                    _0x536bx44++;
                    _particles[j].x = _0x536bx68 + i * 40 + _0x536bx44 * _0x536bx69;
                    if (_0x536bx44 > 10) {
                        _0x536bx44 = 0;
                        _0x536bx55 += _0x536bx69
                    }
                    ;_particles[j].y = _0x536bx55 + Math.random()
                }
            }
        }
        ;this.clearVelocities()
    }
    ;
    this.initializeWithStripes = function() {
        for (var i = 0; i < _particleCount; i++) {
            _particles[i].x = (i / _particleCount) * (_width * _particleTypeCount) - (_particles[i].type / _particleTypeCount) * (_width * _particleTypeCount);
            _particles[i].y = (_particles[i].type / _particleTypeCount + (0.5 / _particleTypeCount)) * _height;
            _particles[i].y += Math.random()
        }
        ;this.clearVelocities()
    }
    ;
    this.initializeRandomOnBottom = function() {
        for (var i = 0; i < _particleCount; i++) {
            _particles[i].x = Math.random() * _width;
            _particles[i].y = _height
        }
        ;this.clearVelocities()
    }
    ;
    this.clearVelocities = function() {
        for (var i = 0; i < _particleCount; i++) {
            _particles[i].xv = 0.0;
            _particles[i].yv = 0.0
        }
    }
    ;
    this.arrowRight = function(_0x536bx54, _0x536bx55) {
        this.printGenes()
    }
    ;
    this.initialize()
}
document.onkeydown = function(_event) {
    if (_event.keyCode === 37) {
        clusters.arrowLeft()
    }
    ;if (_event.keyCode === 39) {
        clusters.arrowRight()
    }
}
;
document.onmousedown = function(_event) {
    clusters.mouseDown(_event.pageX, _event.pageY)
}
;
document.onmousemove = function(_event) {
    clusters.mouseMove(_event.pageX, _event.pageY)
}
;
document.onmouseup = function(_event) {
    clusters.mouseUp(_event.pageX, _event.pageY)
}
