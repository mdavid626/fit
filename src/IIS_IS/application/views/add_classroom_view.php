<div id="search">
	<h1><?php echo $page_title;?></h1>
	<div id="body">
            <?php
                echo form_open($submit, 'id=main_form');
                echo form_hidden('add_flag', '1');
                
                $fields = array('Číslo miestnosti'=>'room_id', 'Typ miestnosti'=>'room_type',
                                'Kapacita'=>'room_capacity', 'Vybavenie'=>'room_equip'
                                );
                
                if(isset($editables))
                {
                    echo form_fieldset();
                    echo form_label('Číslo miestnosti');
                    $attr = array('name' => 'room_id', 'id' => 'room_id', 'value' => $editables['id_miest']);
                    echo form_input($attr);
                    echo form_fieldset_close();
                    
                    echo form_fieldset();
                    echo form_label('Typ miestnosti');
                    $attr = array('name' => 'room_type', 'id' => 'room_type', 'value' => $editables['typ_miest']);
                    echo form_input($attr);
                    echo form_fieldset_close();
                    
                    echo form_fieldset();
                    echo form_label('Kapacita');
                    $attr = array('name' => 'room_capacity', 'id' => 'room_capacity', 'value' => $editables['kapacita']);
                    echo form_input($attr);
                    echo form_fieldset_close();
                    
                    echo form_fieldset();
                    echo form_label('Vybavenie');
                    $attr = array('name' => 'room_equip', 'id' => 'room_equip', 'value' => $editables['spec_vyb']);
                    echo form_input($attr);
                    echo form_fieldset_close();
                    
                }else
                {
                    
                    foreach ($fields as $key => $value) {
                        echo form_fieldset();
                        echo form_label($key);
                        $attr = array('name' => $value, 'id' => $value, 'value' => set_value($value));
                        echo form_input($attr);
                        echo form_fieldset_close();
                    }
                }
                
                echo form_submit('submit', $action);
                echo form_close();
                
            ?>
            <?php echo validation_errors('<p class="error">'); ?>
	</div>
</div>