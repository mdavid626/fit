<div id="login_form">

    <h1>Prihlásenie</h1>
    
    <div id="body">
        <?php 
        echo form_open('login/validate');
        echo form_label('Prihlasovacie meno');
        echo form_input('username', set_value('username'), 'id="username"');
        echo form_label('Heslo');
        echo form_password('password', '');
        echo form_submit('submit', 'Prihlásiť');
        echo form_close();
        ?>
        
        <?php if ($flag) { ?>
        <p class="error">Nesprávne užívateľské meno alebo heslo!</p>
        <?php } ?>
    </div>
    
    <script>
        $("#username").focus();
    </script>

</div><!-- end login_form-->