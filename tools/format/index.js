// This is schizophrenic and is only here because the tooling for hjson is bad and keeping comments only exists for
// the js function
// Just run `node index.js` to format everything
var Hjson = require('hjson');

var fs = require('fs');
var path = require('path');

var walk = function(dir, done) {
  var results = [];
  fs.readdir(dir, function(err, list) {
    if (err) return done(err);
    var pending = list.length;
    if (!pending) return done(null, results);
    list.forEach(function(file) {
      file = path.resolve(dir, file);
      fs.stat(file, function(err, stat) {
        if (stat && stat.isDirectory()) {
          walk(file, function(err, res) {
            results = results.concat(res);
            if (!--pending) done(null, results);
          });
        } else {
          results.push(file);
          if (!--pending) done(null, results);
        }
      });
    });
  });
};

walk("../../binaries/data", function(err, results) {
  if (err) throw err;
  // then hjson format it?
  for (const file_name of results) {
    if (!file_name.endsWith(".hjson")) {
        continue;
    }
    console.log(file_name);
    fs.readFile(file_name, 'utf8', (err, data) => {
        if (err) {
            console.error(err);
            return;
        }
        var text = Hjson.rt.parse(data);
        var output = Hjson.rt.stringify(text, {
            bracesSameLine: false
        });

        fs.writeFile(file_name, output, 'utf8', (err) => {
            if (err) {
                console.error("Write Error:", err);
                return;
            }
        });
    });
  }
});
