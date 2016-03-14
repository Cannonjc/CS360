angular.module('comment', [])
.controller('MainCtrl', [
  '$scope',
  function($scope){
    $scope.test = 'Hello world!';
  }
]);
