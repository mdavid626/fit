
<div id="admin_container">
    
    <script type="text/javascript">
        function confirm_delete()
        {
            return confirm("Naozaj chcete odstrániť vybrané učenbe?");
        }
        
    </script>

	<div id="admin_body">
        <p><?php echo anchor('management/add_classroom', 'Pridať novú učebňu'); ?><p>
        <?php if ($results_admin) { ?>
        <h3>Dostupné učebne</h3>
            <div id="admin_search_results">
                <?php if ($query) { ?>
                                    
                    <table border="0" cellpadding="4" cellspacing="0">
                    <thead>
                    <tr>
                    <th>Odstrániť</th><th>Miestnosť</th><th>Typ miestnosti</th><th>Kapacita</th><th>Vybavenie</th><th>Voľby</th></tr>
                    </thead>
                    <tbody>
                    
                    <?php 
                        echo form_open($action='management/del_classroom', 'id="main_form" onsubmit="return confirm_delete()"');
                        
                        foreach ($query as $row)
                        {
                            echo '<tr>';
                            echo '<td>', form_checkbox('del_cl_chk[]', $row->id_miest, set_checkbox('del_cl_chk', $row->id_miest, 0));
                            echo '<td>', $row->id_miest, '</td>';
                            echo '<td>', $row->typ_miest, '</td>';
                            echo '<td>', $row->kapacita, '</td>';
                            echo '<td>', $row->spec_vyb, '</td>';
                            echo '<td>', anchor('management/edit_classroom/'.$row->id_miest, 'zmeniť'),'</td>';
                            echo '</tr>';
                        }
                    ?>
                    
                    </tbody>
                    </table>
                <p><?php 
                         //$js = 'onClick="confirm(\'Naozaj chcete odstrániť vybrané učenbne?\')';
                         echo form_submit('submit', 'Ostrániť vybrané učebne');
                         echo form_close();
                    //echo anchor('site/del_classroom', 'Ostrániť vybrané učebne');?></p>
                <?php } else { ?>
                    Žiadna učebna nevyhovuje zadaným kritériám!
                <?php } ?>
            </div>
        <?php } ?>
        
        
	</div>
</div>