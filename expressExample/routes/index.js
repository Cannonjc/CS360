var express = require('express');
var router = express.Router();
var fs = require('fs');

/* GET home page. */
router.get('/', function(req, res, next) {
  res.sendFile('cityFinder.html', {
    root: 'public'
  });
});

router.get('/getcity', function(req, res) {
  var myRe = new RegExp("^" + req.query.q);
  console.log(myRe);
  fs.readFile(__dirname + '/cities.dat.txt', function(err, data) {
    if (err) throw err;
    var cities = data.toString().split("\n");
    var jsonresult = [];
    for (var i = 0; i < cities.length; i++) {
      var result = cities[i].search(myRe);
      if (result != -1) {
        jsonresult.push({
          city: cities[i]
        });
        console.log(cities[i]);
      }
    }
    res.status(200).json(jsonresult);
  });
});

router.get('/test1', function(req,res) {
  res.sendFile('test1.html', {
    root: 'public'
  });
});

router.get('/test2', function(req,res) {
  res.sendFile('test2.txt', {
    root: 'public'
  });
});

router.get('/test3', function(req,res) {
  res.sendFile('test3.gif', {
    root: 'public'
  });
});

router.get('/test4', function(req,res) {
  res.sendFile('test4.jpg', {
    root: 'public'
  });
});

module.exports = router;
