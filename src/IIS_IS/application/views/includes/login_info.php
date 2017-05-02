<div id="login_info">
Vitajte, <?php 
    if (!empty($fname)) 
    {
        echo $fname, '!';
    }
    else 
    {
        echo 'hosť!';
    }
    
    echo ' [ ';
    //echo '</br>';
    
    if (!empty($role) && $role != M_UNAUTH)
    {
        echo anchor('login/logout', 'Odhlásenie ]');
    }
    else
    {
        echo anchor('login', 'Prihlásenie ]');
    }
?>
</div>