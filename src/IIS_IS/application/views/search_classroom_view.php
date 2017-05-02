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
                if( isset($introduction_flag))
                { ?>
            <p>Pre zobrazenie dostupných učebien, kliknete na tlačítko Vyhľadať</p>
            <?php
                }
            ?>
    
        <?php
        echo form_open($submit, 'id="main_form"');
        echo form_hidden('search_flag', '1');
        ?>
        
        <fieldset>
        <?php
        echo form_checkbox('by_room_id', 'accept', set_checkbox('by_room_id', 'accept', 0), 'id="by_room_id"');
        echo form_label('Identifikačné číslo');
        
        $attr = array('name' => 'room_id', 'id' => 'room_id', 'value' => set_value('room_id'));
        echo form_input($attr);
        ?>
        </fieldset>
        
        <fieldset>
        <?php
        echo form_checkbox('by_miesto', 'accept', set_checkbox('by_miesto', 'accept', 0), 'id="by_miesto"');
        echo form_label('Typ miesta');
    
        echo form_dropdown('miesto', $typ_miesta_dd, set_value('miesto', ''), 'id="miesto"');
        ?>
        </fieldset> 
    
        <fieldset>
        <?php
        echo form_checkbox('by_kapacita', 'accept', set_checkbox('by_kapacita', 'accept', 0), 'id="by_kapacita"');
        echo form_label('Min. kapacita');
        
        $attr = array('name' => 'kapacita', 'id' => 'kapacita', 'value' => set_value('kapacita'));
        echo form_input($attr);
        ?>
        </fieldset> 
        
        <fieldset>
        <?php
        echo form_checkbox('by_vybavenie', 'accept', set_checkbox('by_vybavenie', 'accept', 0), 'id="by_vybavenie"');
        echo form_label('Vybavenie');
        
        $attr = array('name' => 'vybavenie', 'id' => 'vybavenie', 'value' => set_value('vybavenie'));
        echo form_input($attr);
        ?>
        </fieldset>
        
        <fieldset>
        <?php
        echo form_checkbox('by_datum1', 'accept', set_checkbox('by_datum1', 'accept', 0), 'id="by_datum1"');
        echo form_label('Volné od (dd.mm.rrrr hh:mm)');
        
        $datum1 = array('name' => 'datum1', 'id' => 'datum1', 'value' => set_value('datum1'));
        echo form_input($datum1);
        ?>
        </fieldset>   
        
        <fieldset>
        <?php
        echo form_checkbox('by_datum2', 'accept', set_checkbox('by_datum2', 'accept', 0), 'id="by_datum2"');
        echo form_label('Volné do (dd.mm.rrrr hh:mm)');
    
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
                                    
                    <table border="0" cellpadding="4" cellspacing="0">
                    <thead>
                    <tr>
                    <th>Miestnosť</th><th>Typ miestnosti</th><th>Kapacita</th><th>Vybavenie</th></tr>
                    </thead>
                    <tbody>
                    
                    <?php 
                        foreach ($query as $row)
                        {
                            echo '<tr>';
                            echo '<td>', $row->id_miest, '</td>';
                            echo '<td>', $row->typ_miest, '</td>';
                            echo '<td>', $row->kapacita, '</td>';
                            echo '<td>', $row->spec_vyb, '</td>';
                            echo '</tr>';
                        }
                    ?>
                    
                    </tbody>
                    </table>
                    
                <?php } else { ?>
                    Žiadna učebna nevyhovuje zadaným kritériám!
                <?php } ?>
            </div>
        <?php } ?>
        
    </div>
    
</div>