<div id="add">
    
    <script>
        
        function clear_form(conf_dialog)
        {
            if (conf_dialog && !confirm('Naozaj chcete vymazať formulár?'))
            {
                return;
            }
            
            $('form').clearForm();
        }
        
	</script>

	<h1>Pridať zamestnanca</h1>

	<div id="body">
        <fieldset>
	<?php
        echo form_open('management/add_user', 'id="main_form"');
        echo form_hidden('post_flag', '1'); 
        
        echo form_label('Rodné číslo*');        
        echo form_input('r_cislo', set_value('r_cislo'), 'id="r_cislo"');
        
        echo form_label('Priezvisko*');        
        echo form_input('priezvisko', set_value('typriezviskop'), 'id="priezvisko"');
        
        echo form_label('Meno*');        
        echo form_input('meno', set_value('meno'), 'id="meno"');
        
        echo form_label('Titul');        
        echo form_input('titul', set_value('titul'), 'id="titul"');
        
        echo form_label('Zobrazovacie meno*');        
        echo form_input('zobraz_meno', set_value('zobraz_meno'), 'id="zobraz_meno"');
        
        echo form_label('Prihlasovacie meno*');        
        echo form_input('username', set_value('username'), 'id="username"');
        
        echo form_label('Heslo*');        
        echo form_input('heslo', set_value('heslo'), 'id="heslo"');
        
        echo form_label('Pracovný pomer*');        
        echo form_input('prac_pomer', set_value('prac_pomer'), 'id="prac_pomer"');
 
    ?>
    <p class="small">* označuje povinné položky</p>
    </fieldset>
    
    <?php
      
        echo form_submit('submit', 'Pridať');
        
        $js = 'onClick="clear_form(true)"';
        echo form_button('reset', 'Vymazat formulár', $js);
        
        echo form_close();
    ?>
        
    <?php echo validation_errors('<p class="error">'); ?>
    
    <?php if ($zam_error == 1) { ?>
    
        <p class="error">Zamestnanec už v databázi existuje!</p>
    
    <?php }  ?>
    
    
    <?php if ($results) { ?>

        <?php if (!$query) { ?>
        
            <p class="error">Nastal problém pri vložení data do databáze. Prosím skúste znova!</p>
        
        <?php } else { ?>
            
            <p class="success">Zamestnanec je úspešne evidovaný v databázi.</p>
            
            <script>
                clear_form(false);
            </script>
            
        <?php } ?>
    
    <?php } ?>
        
    </div>
</div>