<?php $this->load->view('includes/header'); ?>

<div class="menu">
<?php
    $this->load->view('includes/menu');
    
    if ($role == M_USER)
    {
        $this->load->view('includes/menu_user');
    }
    if ($role == M_ADMIN)
    {
        $this->load->view('includes/menu_admin');
    }
?>
</ul>
</div>

<div id="view_container">
<?php
    if(is_array($main_content)) 
    {
        foreach ($main_content as $value) {
            $this->load->view($value);
        } 
    }else
    {
        $this->load->view($main_content); 
    }
?>
</div>

<?php $this->load->view('includes/footer'); ?>