<div id="search">

    <script>
    
        $(function() {
            $("#datum1").datetimepicker({ dateFormat: 'dd.mm.yy' });
            $("#datum2").datetimepicker({ dateFormat: 'dd.mm.yy' });
            
            init_form();
            
            // click
            $("input:checkbox").click(function () {
                var item = $(this).attr("id").replace("by_", "#");
                $(item).attr("disabled", !$(this).is(":checked"));
            });
        });
        
        function init_form()
        {
            $("input:checkbox").each(function() {
                var item = $(this).attr("id").replace("by_", "#");
                $(item).attr("disabled", !$(this).is(":checked"));
            });
        }
        
        function clear_form()
        {
            $('form').clearForm();
            init_form();
        }
        
	</script>
    
	<h1><?php echo $page_title?></h1>

	<div id="body">
        
        <?php
        echo form_open('search/reservation', 'id="main_form"');
        echo form_hidden('search_flag', '1');
        ?>
        
        <fieldset>
        <?php
        echo form_checkbox('by_room_id', 'accept', set_checkbox('by_room_id', 'accept', 0), 'id="by_room_id"');
        echo form_label('Identifikačné číslo učebňe');
        echo form_input('room_id', set_value('room_id'), 'id="room_id"');
        ?>
        </fieldset>
        
        <br />
        
        <fieldset>
        <?php
        echo form_checkbox('by_datum1', 'accept', set_checkbox('by_datum1', 'accept', 0), 'id="by_datum1"');
        echo form_label('Termín od (dd.mm.rrrr hh:mm)');
        
        $datum1 = array('name' => 'datum1', 'id' => 'datum1', 'value' => set_value('datum1'));
        echo form_input($datum1);
        ?>
        </fieldset>   
        
        <fieldset>
        <?php
        echo form_checkbox('by_datum2', 'accept', set_checkbox('by_datum2', 'accept', 0), 'id="by_datum2"');
        echo form_label('Termín do (dd.mm.rrrr hh:mm)');
    
        $datum2 = array('name' => 'datum2', 'id' => 'datum2', 'value' => set_value('datum2'));
        echo form_input($datum2);
        ?>
        </fieldset>   
        
        <?php
        echo form_submit('submit', 'Vyhľadať');
        
        $js = 'onClick="clear_form()"';
        echo form_button('reset', 'Zrušiť všetky filtre', $js);
        
        echo form_close();
        ?>
        
        <?php echo validation_errors('<p class="error">'); ?>
        
        <?php if ($results) { ?>
        <h3>Výsledky vyhľadávania</h3>
            <div id="search_results">
                <?php if ($query) { ?>
                    
                    <table border="0" cellpadding="5" cellspacing="0">
                    <thead>
                    <tr>
                    <th>Miestnosť</th><th>Typ udalosti</th><th>Rezervujúci</th><th>Čas začiatku</th><th>Čas ukončenia</th></tr>
                    </thead>
                    <tbody>
                    
                    <?php 
                        foreach ($query as $row)
                        {
                            echo '<tr>';
                            echo '<td>', $row->id_miest, '</td>';
                            echo '<td>', $row->typ_udal, '</td>';
                            echo '<td>', $row->zobraz_meno, '</td>';
                            echo '<td>', $row->cas_zac, '</td>';
                            echo '<td>', $row->cas_ukon, '</td>';
                            echo '</tr>';
                        }
                    ?>
                    
                    </tbody>
                    </table>
                    
                <?php } else { ?>
                    Žiadna rezervácia nevyhovuje zadaným kritériám!
                <?php } ?>
            </div>
        <?php } ?>
        
	</div>
</div>