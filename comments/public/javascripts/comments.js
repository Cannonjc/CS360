$(document).ready(function(){
    $("#serialize").click(function(){
        var myobj = {Name:$("#Name").val(),Comment:$("#Comment").val()};
        jobj = JSON.stringify(myobj);
        $("#json").text(jobj);
        console.log("serializing");

      	var url = "comment";
      	$.ajax({
        	url:url,
        	type: "POST",
        	data: jobj,
        	contentType: "application/json; charset=utf-8",
        	success: function(data,textStatus) {
            	$("#done").html(textStatus);
        	}
      	})
    });
    $("#getThem").click(function() {
      $.getJSON('comments', function(data) {
        console.log(data);
        var everything = "<ul>";
        if ($("#Password").val() == "admin") {
          $("#json").text("Please enter the correct password to get the accurate results");
          for(var comment in data) {
          com = data[comment];
          everything += "<li>Name: " + com.Name + " -- Comment: " + com.Comment + "</li>";
          }
        }
        else {
          $("#json").text("");
          for(var comment in data) {
          com = data[comment];
          everything += "<li>Name: " + com.Name.split("").reverse().join("") + " -- Comment: " + com.Comment.split("").reverse().join("") + "</li>";
          }
        }
        everything += "</ul>";
        $("#comments").html(everything);
      })
    });
});

