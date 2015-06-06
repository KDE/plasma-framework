function getProperty (prop, key) {
    return _dict_[key][prop];
}
Ts.setcall("get-case", getProperty);

function lowerCaseFirstLetter(string)
{
    return string.charAt(0).toLowerCase() + string.slice(1);
}

_dict_ = {};
function addDictCases (key, kil, nau, gal, ina, vie) {
    if (!_dict_[key])
        _dict_[key] = {};
    _dict_[key]["vardininkas"] = lowerCaseFirstLetter(key);
    _dict_[key]["kilmininkas"] = lowerCaseFirstLetter(kil);
    _dict_[key]["naudininkas"] = lowerCaseFirstLetter(nau);
    _dict_[key]["galininkas"] = lowerCaseFirstLetter(gal);
    _dict_[key]["įnagininkas"] = lowerCaseFirstLetter(ina);
    _dict_[key]["vietininkas"] = lowerCaseFirstLetter(vie);
    _dict_[key]["Kilmininkas"] = kil
    _dict_[key]["Naudininkas"] = nau;
    _dict_[key]["Galininkas"] = gal;
    _dict_[key]["Įnagininkas"] = ina;
    _dict_[key]["Vietininkas"] = vie;
}

function addCapitalDictCases (key, kil, nau, gal, ina, vie) {
    if (!_dict_[key])
        _dict_[key] = {};
    _dict_[key]["vardininkas"] = key;
    _dict_[key]["kilmininkas"] = kil;
    _dict_[key]["naudininkas"] = nau;
    _dict_[key]["galininkas"] = gal;
    _dict_[key]["įnagininkas"] = ina;
    _dict_[key]["vietininkas"] = vie;
    _dict_[key]["Kilmininkas"] = kil
    _dict_[key]["Naudininkas"] = nau;
    _dict_[key]["Galininkas"] = gal;
    _dict_[key]["Įnagininkas"] = ina;
    _dict_[key]["Vietininkas"] = vie;
}

function dynamicSetCases (kil, nau, gal, ina, vie) {
    addDictCases(Ts.msgstrf(), kil, nau, gal, ina, vie);
    Ts.fallback();
}

Ts.setcall("set-cases", dynamicSetCases);
Ts.load("plasmoids")
