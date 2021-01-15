//-------------------------------------------------------------for navbar------------------------------------------------------------

function w3_open() {
	document.getElementById("main").style.marginLeft = "25%";
	document.getElementById("graph-wrapper" ).style.marginLeft = "5%";
	document.getElementById("Trends" ).style.marginLeft = "5%";
	document.getElementById("Overview" ).style.marginLeft = "7%";
	document.getElementById("myInput" ).style.marginLeft = "8%";
	document.getElementById("myTable").style.marginLeft = "8%";
    document.getElementById("mySidebar").style.width = "25%";
    document.getElementById("mySidebar").style.display = "block";
    document.getElementById("openNav").style.display = 'none';
  }
  function w3_close() {
	document.getElementById("main").style.marginLeft = "0%";
	document.getElementById("Trends" ).style.marginLeft = "0%";
	document.getElementById("Overview" ).style.marginLeft = "0%";
	document.getElementById("myInput" ).style.marginLeft = "0%";
	document.getElementById("graph-wrapper").style.marginLeft = "0%";
	document.getElementById("myTable").style.marginLeft = "0%";
    document.getElementById("mySidebar").style.display = "none";
    document.getElementById("openNav").style.display = "inline-block";
  }

  var sidebar = document.getElementById("mySidebar");

  window.onclick = function(event) {
	if (event.target == sidebar) {
	  sidebar.style.display = "none";
	}
  }

  $(document).ready(function () {


	var graphData = [{
			
			data: [ [6, 13], [7, 27], [8, 0], [9, 0], [10, 3], [11, 0], [12, 15] ],
			color: '#71c73e'
		}, {
			
			data: [ [6, 183], [7, 213], [8, 170], [9, 203], [10, 197], [11, 201] ],
			color: '#77b7c5',
			points: { radius: 4, fillColor: '#77b7c5' }
		}
	];

	$.plot($('#graph-lines'), graphData, {
		series: {
			points: {
				show: true,
				radius: 5
			},
			lines: {
				show: true
			},
			shadowSize: 0
		},
		grid: {
			color: '#646464',
			borderColor: 'transparent',
			borderWidth: 20,
			hoverable: true
		},
		xaxis: {
			tickColor: 'transparent',
			tickDecimals: 2
		},
		yaxis: {
			tickSize: 10
		}
	});
//-------------------------------------------------------------for interactive graph------------------------------------------------------------

	$.plot($('#graph-bars'), graphData, {
		series: {
			bars: {
				show: true,
				barWidth: .9,
				align: 'center'
			},
			shadowSize: 0
		},
		grid: {
			color: '#646464',
			borderColor: 'transparent',
			borderWidth: 20,
			hoverable: true
		},
		xaxis: {
			tickColor: 'transparent',
			tickDecimals: 2
		},
		yaxis: {
			tickSize: 10
		}
	});


	$('#graph-bars').hide();

	$('#lines').on('click', function (e) {
		$('#bars').removeClass('active');
		$('#graph-bars').fadeOut();
		$(this).addClass('active');
		$('#graph-lines').fadeIn();
		e.preventDefault();
	});

	$('#bars').on('click', function (e) {
		$('#lines').removeClass('active');
		$('#graph-lines').fadeOut();
		$(this).addClass('active');
		$('#graph-bars').fadeIn().removeClass('hidden');
		e.preventDefault();
	});


	function showTooltip(x, y, contents) {
		$('<div id="tooltip">' + contents + '</div>').css({
			top: y - 16,
			left: x + 20
		}).appendTo('body').fadeIn();
	}

	var previousPoint = null;

	$('#graph-lines, #graph-bars').bind('plothover', function (event, pos, item) {
		if (item) {
			if (previousPoint != item.dataIndex) {
				previousPoint = item.dataIndex;
				$('#tooltip').remove();
				var x = item.datapoint[0],
					y = item.datapoint[1];
					showTooltip(item.pageX, item.pageY, y + ' visitors at ' + x + 'hrs');
			}
		} else {
			$('#tooltip').remove();
			previousPoint = null;
		}
	});

});

//-----------------------------------------------------------for overviews------------------------------------------------------------------------

function myFunction() {
    var input, filter, table, tr, td, i, txtValue;
    input = document.getElementById("myInput");
    filter = input.value.toUpperCase();
    table = document.getElementById("myTable");
    tr = table.getElementsByTagName("tr");
    for (i = 0; i < tr.length; i++) {
      td = tr[i].getElementsByTagName("td")[0];
      if (td) {
        txtValue = td.textContent || td.innerText;
        if (txtValue.toUpperCase().indexOf(filter) > -1) {
          tr[i].style.display = "";
        } else {
          tr[i].style.display = "none";
        }
      }       
    }
  }

  //------------------------------------------------------------for user settings----------------------------------------------------------------
   
  var modal = document.getElementById("myModal");
  
  var btn = document.getElementById("settings");
  
  var span = document.getElementsByClassName("close")[0];
  
  btn.onclick = function() {
	modal.style.display = "block";
  }
  span.onclick = function() {
	modal.style.display = "none";
  }
  
  // When the user clicks anywhere outside of the modal, close it
  window.onclick = function(event) {
	if (event.target == modal) {
	  modal.style.display = "none";
	}
  }