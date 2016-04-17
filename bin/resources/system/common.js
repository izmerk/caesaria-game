function _t(text) { return engine.translate(text) }

function _u(text) {
  if (text[0] == "#")
    return text;

  return "##"+text+"##"
}

function _ut(text) { return _t(_u(text)) }

var _format = function() {
    var formatted = arguments[0]
    for (var arg in arguments) {
                if(arg==0)
                    continue
        formatted = formatted.replace("{" + (arg-1) + "}", arguments[arg])
    }
    return formatted
};
