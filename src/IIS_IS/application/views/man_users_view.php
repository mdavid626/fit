<div id="search">
    <script type="text/javascript">
        function confirm_delete()
        {
            return confirm("Naozaj chcete odstrániť vybrané zamestnance?");
        }
    </script>
    
    <h1>Zamestnance</h1>

    <div id="body">

        <?php if ($query_result) { ?>

            <table border="0" cellpadding="4" cellspacing="0">
            <thead>
            <tr>
            <th>Odstrániť</th><th>Priezvisko</th>
            <th>Meno</th><th>Titul</th><th>Rodné č.</th><th>Prihlas. meno</th><th>Prac. pomer</th></tr>
            </thead>
            <tbody>
                
            <?php 
                echo form_open($action='management/del_user', 'id="main_form" onsubmit="return confirm_delete()"');

                foreach ($query_result as $row)
                {
                    echo '<tr>';
                    echo '<td>', form_checkbox('del_cl_chk[]', $row->r_cislo, set_checkbox('del_cl_chk', $row->r_cislo, 0));
                    echo '<td>', $row->priezvisko, '</td>';
                    echo '<td>', $row->meno, '</td>';
                    echo '<td>', $row->titul, '</td>';
                    echo '<td>', $row->r_cislo, '</td>';
                    echo '<td>', $row->username, '</td>';
                    echo '<td>', $row->prac_pomer, '</td>';
                    echo '</tr>';
                }
            ?>

            </tbody>
            </table>
        <?php 
                 //$js = 'onClick="confirm(\'Naozaj chcete odstrániť vybrané učenbne?\')';
                 echo form_submit('submit', 'Ostrániť vybraných');
                 echo form_close(); ?>
                 <p><?php echo anchor('management/add_user', 'Pridať zamestnanca');?></p>
        <?php } else { ?>
            Žiadna učebna nevyhovuje zadaným kritériám!
        <?php } ?>


    </div>
</div>