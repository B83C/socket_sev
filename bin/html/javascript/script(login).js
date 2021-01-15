function myFunction() {
    var element = document.body;
    element.classList.toggle("dark-mode");

    var element2 = document.span;
    element2.classList.toggle("dark-mode");

 }

 function check(form)
{

if(form.userid.value == "peiteng" && form.pswrd.value == "123")
 {
   window.open("homepage.html")
 }
else
{
  alert("Error Password or Username")
 }
}