function getProperty (prop, key) {
    return _dict_[key][prop];
}
Ts.setcall("get-case", getProperty);

function upperCaseFirstLetter(string)
{
    return string.charAt(0).toUpperCase() + string.slice(1);
}

_dict_ = {};
function addDictCases (key, kil, gal) {
    if (!_dict_[key])
        _dict_[key] = {};
    _dict_[key]["vardininkas"] = key;
    _dict_[key]["kilmininkas"] = kil;
    _dict_[key]["galininkas"] = gal;
    _dict_[key]["Kilmininkas"] = upperCaseFirstLetter(kil)
    _dict_[key]["Galininkas"] = upperCaseFirstLetter(gal);
}

function dynamicSetCases (kil, gal) {
    addDictCases(Ts.msgstrf(), kil, gal);
    Ts.fallback();
}

Ts.setcall("set-cases", dynamicSetCases);
Ts.load("plasmoids")
