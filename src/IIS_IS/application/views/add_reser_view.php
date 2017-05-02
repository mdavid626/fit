<div id="add">
    
    <script>
    
        $(function() {
            $("#datum1").datetimepicker({ dateFormat: 'dd.mm.yy' });
            $("#datum2").datetimepicker({ dateFormat: 'dd.mm.yy' });
        });
        
        function clear_form(conf_dialog)
        {
            if (conf_dialog && !confirm('Naozaj chcete vymazať formulár?'))
            {
                return;
            }
            
            $('form').clearForm();
        }
        
	</script>

	<h1>Pridať rezerváciu</h1>

	<div id="body">
    
    <fieldset>
	<?php
        echo form_open('user/add_reservation', 'id="main_form"');
        echo form_hidden('post_flag', '1'); 
        
        echo form_label('Učebňa*');        
        echo form_dropdown('ucebna', $ucebna_dd, set_value('ucebna', ''), 'id="ucebna"');
        
        echo form_label('Typ udalosti');        
        echo form_input('typ', set_value('typ'), 'id="typ"');
        
        echo form_label('Termín od (dd.mm.rrrr hh:mm)*');
        $datum1 = array('name' => 'datum1', 'id' => 'datum1', 'value' => set_value('datum1'));
        echo form_input($datum1);

        echo form_label('Termín do (dd.mm.rrrr hh:mm)*');
        $datum2 = array('name' => 'datum2', 'id' => 'datum2', 'value' => set_value('datum2'));
        echo form_input($datum2);
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
    
    <?php if ($datum_error == 1) { ?>
    
        <p class="error">Čas začiatku nemôže prekročiť čas ukončenia!</p>
    
    <?php } else if ($datum_error == 2) { ?>
    
        <p class="error">Učebňa v danom termíne je už obsadená!</p>
    
    <?php } ?>
    
    <?php if ($results) { ?>

        <?php if (!$query) { ?>
        
            <p class="error">Nastal problém pri vložení data do databáze. Prosím skúste znova!</p>
        
        <?php } else { ?>
            
            <p class="success">Rezervácia bola úspešne pridaná do databáze.</p>
            
            <script>
                clear_form(false);
            </script>
            
        <?php } ?>
    
    <?php } ?>
        
    </div>
</div>