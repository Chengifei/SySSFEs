/* Copyright (c) 2017 by Yifei Zheng
 * This file is part of ATOM project.
 * 
 * This file defines few routines that used for parse configurations.
*/

"use strict";

function AddRule(textbox) {
    var content = document.getElementById('arg2').value;
    textbox.value += "Space.addRule('" + content + "')\n";
}

function RuleAddCond() {
    var config = document.getElementById('rule-config');
    var vals = config.children[0];
    vals.innerHTML += 'Else if <input type="text" id="arg1" placeholder="Blank for \'else\'"><br><input type="text" id="arg2"><br>';
}

function AddWatch(textbox) {
    var box = document.getElementById('arg1');
    var name = box.value;
    if (validate_objname(name)) {
        var types = $('#arg2').find('button');
        var save = [];
        for (var i = 0; i < types.length; i++) { 
            if (types[i].classList.contains('active'))
                save.push(types[i].innerText);
        }
        watches[name] = save;
        textbox.value += "Space.addWatch(" + name + ")\n";
    }
    else
        box.style.borderColor = "red";
}

function AddObject(textbox) {
    var name = document.getElementById('arg1').value;
    if (validate_objname(name))
        textbox.value += "Space.addObject(" + name + ")\n";
}

function validate_objname(name) {
    var regex = /^\w+$/;
    if (name && regex)
        return true;
    return false;
}
