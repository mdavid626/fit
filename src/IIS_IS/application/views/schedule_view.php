<div id="search">

    <script>
    
        $(function() {
            $("#datum1").datetimepicker({ dateFormat: 'dd.mm.yy' });
            $("#datum2").datetimepicker({ dateFormat: 'dd.mm.yy' });
            
            init_form();
            
            // click
            /*$("input:radio").click(function () {
                init_form();
            });*/
            $("input:checkbox").click(function () {
                var item = $(this).attr("id").replace("by_", "#");
                $(item).attr("disabled", !$(this).is(":checked"));
            });
        });
        
        function init_form()
        {
            /*$("input:radio").each(function() {
                var item = $(this).attr("id").replace("by_", "#");
                
                if (item == "#datum")
                {
                    item += "1, " + item + "2";
                }
                
                $(item).attr("disabled", !$(this).is(":checked"));
            });*/
            
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
        echo form_open('search/schedule', 'id="main_form"');
        echo form_hidden('post_flag', '1');
        ?>
        
        <fieldset>
        <?php
        //echo form_radio('radio', 'by_ucebna', set_radio('radio', 'by_ucebna'), 'id="by_ucebna"');
        echo form_checkbox('by_ucebna', 'accept', set_checkbox('by_ucebna', 'accept', 0), 'id="by_ucebna"');
        echo form_label('Učebňa');        
        echo form_dropdown('ucebna', $ucebna_dd, set_value('ucebna', ''), 'id="ucebna"');
        ?>
        </fieldset>
        
        <br />
        
        <fieldset>
        <?php
        //echo form_radio('radio', 'by_obor', set_radio('radio', 'by_obor'), 'id="by_obor"');
        echo form_checkbox('by_obor', 'accept', set_checkbox('by_obor', 'accept', 0), 'id="by_obor"');
        echo form_label('Obor');
        echo form_dropdown('obor', $obor_dd, set_value('obor', ''), 'id="obor"');
        ?>
        </fieldset>
        
        <fieldset>
        <?php
        //echo form_radio('radio', 'by_rocnik', set_radio('radio', 'by_rocnik'), 'id="by_rocnik"');
        echo form_checkbox('by_rocnik', 'accept', set_checkbox('by_rocnik', 'accept', 0), 'id="by_rocnik"');
        echo form_label('Ročník');
        echo form_dropdown('rocnik', $rocnik_dd, set_value('rocnik', ''), 'id="rocnik"');
        ?>
        </fieldset>
        
        <fieldset>
        <?php
        //echo form_radio('radio', 'by_datum', set_radio('radio', 'by_datum'), 'id="by_datum"');
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
        echo form_submit('submit', 'Ukázať');
        
        $js = 'onClick="clear_form()"';
        echo form_button('reset', 'Zrušiť všetky filtre', $js);
        
        echo form_close();
        ?>
        
        <?php echo validation_errors('<p class="error">'); ?>
        
        <?php if ($results) { ?>
        <h3>Výsledky</h3>
            <div id="search_results">
                <?php if ($query) { ?>
                                    
                    <table border="0" cellpadding="7" cellspacing="0">
                    <thead>
                    <tr>
                    <th>Predmet</th><th>Vyučujúci</th><th>Typ vyučovania</th><th>Počet žiakov</th>
                    
                    <?php
                        if ($ucebna_h)
                        {
                            echo '<th>Miestnosť</th>';
                        }
                        
                        if ($obor_h)
                        {
                            echo '<th>Obor</th>';
                        }
                        
                        if ($rocnik_h)
                        {
                            echo '<th>Ročník</th>';
                        }
                    ?>
                    
                    <th>Čas začiatku/th><th>Čas ukončenia</th></tr>
                    </thead>
                    <tbody>
                    
                    <?php 
                        foreach ($query as $row)
                        {
                            echo '<tr>';
                            echo '<td>', $row->skratka_predmetu, '</td>';
                            echo '<td>', $row->zobraz_meno, '</td>';
                            echo '<td>', $row->typ_vyuky, '</td>';
                            echo '<td>', $row->pocet_reg_ziak, '</td>';
                            
                            if ($ucebna_h)
                            {
                                echo '<td>', $row->id_miest, '</td>';
                            }
                            
                            if ($obor_h)
                            {
                                echo '<td>', $row->stud_obor, '</td>';
                            }
                            
                            if ($rocnik_h)
                            {
                                echo '<td>', $row->rocnik, '</td>';
                            }

                            echo '<td>', $row->cas_zac, '</td>';
                            echo '<td>', $row->cas_ukon, '</td>';
                            echo '</tr>';
                        }
                    ?>
                    
                    </tbody>
                    </table>
                    
                <?php } else { ?>
                    Žiadny rozvrh nevyhovuje zadaným kritériám!
                <?php } ?>
            </div>
        <?php } ?>
        
    </div>
</div>