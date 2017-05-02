<div id="add">
	<h1><?php echo $page_title?></h1>
	<div id="body">
    
    <fieldset>
    <?php
        echo form_open_multipart('management/man_xml', 'name="import_form", id="import_form"');
        echo form_hidden('import_flag', 'import');
        
        echo form_label('Import tabuľky &quot;Ucebna&quot;');
        echo form_submit('submit', 'Importovať');
        
        echo '<br />';
        echo form_checkbox('overwrite', 'accept', set_checkbox('overwrite', 'accept', 0), 'id="overwrite"');
        echo form_label('Prepísať tabuľku');
              
        echo '<br />';
        echo form_upload('userfile');
           
        
        echo form_close();
    ?> 
    </fieldset>
    
    <fieldset>
	<?php 
        echo form_open('management/man_xml', 'name="export_form", id="export_form"');
        echo form_hidden('export_flag', 'export');
        
        echo form_label('Export tabuľky &quot;Ucebna&quot;');
        echo form_submit('submit', 'Exportovať');
        
        echo form_close();
    ?>
    </fieldset>
    
    <?php 
        if ($upload_error) 
        {
            echo $upload_error;
        }
    ?>
 
    <?php if ($import_status['status'] < 0) { ?>
    
        <p class="error">
        
        <?php 
            switch ($import_status['status'])
            { 
                case -1: echo 'Nastala chyba pri importováni. Prosím, skúste ešte raz!'; break;
                case -2: echo 'Žiadne data nenájdené v importovanom súboru.'; break;
                case -3: echo 'Nasledujúce údaje sú povinné: id_miest, typ_miest, kapacita (', $import_status['data'], ').'; break;
                case -4: echo 'Nasledujúca položka je už v databázi: ', $import_status['data'], '.'; break;
            }
        ?>

        </p>
    
    <?php } else if ($import_status['status'] > 0) { ?>
    
        <p class="success">Importovanie úspešné. Počet pridaných riadkov: <?php echo $import_status['data'] ?>.</p>
        
        <script>
            $('form').clearForm();
        </script>
    
    <?php } ?>
        
	</div>
</div>