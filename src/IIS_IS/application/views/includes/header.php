<!DOCTYPE html>
<html lang="cz">
<head>
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8"> 
	<title><?php echo $page_title;?> | Učebne</title>
	<link rel="stylesheet" href="<?php echo base_url();?>css/style.css" type="text/css" media="screen" />
    <link rel="SHORTCUT ICON" href="<?php echo base_url();?>img/favicon.ico" type="image/x-icon"><script src="http://ajax.googleapis.com/ajax/libs/jquery/1.3.2/jquery.min.js" type="text/javascript" charset="utf-8"></script>
 
    <!-- for jquery -->
    <script src="http://ajax.googleapis.com/ajax/libs/jquery/1.7.1/jquery.min.js" type="text/javascript" charset="utf-8"></script>
    
    <!-- datetimepicker -->
    <script type="text/javascript" src="<?php echo base_url();?>js/ui/jquery-ui-1.8.16.custom.min.js"></script>
    <script type="text/javascript" src="<?php echo base_url();?>js/ui/jquery-ui-timepicker-addon.js"></script>
    <link rel="stylesheet" href="<?php echo base_url();?>css/jquery-theme/jquery-ui-1.8.16.custom.css" />
    
    <!-- other javascript -->
    <script src="<?php echo base_url();?>js/site.js" type="text/javascript" charset="utf-8"></script>
</head>
<body>
<div id="page">

<div id="logo">
    
    <?php $this->load->view('includes/login_info'); ?>
    <h1>Učebne</h1>
    
</div>
