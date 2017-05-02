<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');
require_once 'includes/authtool.php';
require_once 'includes/setuptool.php';

/**
 * Description of manageusers
 *
 * @author xmesza03
 */
class management extends CI_Controller {
    protected $search_filters = 1;
    
    public function index()
    {
        $data = array();//$data['results'] = 0;
        $this->load->model('user_model');
        $data['query_result'] = $this->user_model->get_users();
        AuthTool::load_admin('man_users_view', 'Zamestnance', $data);
    }
    
    public function man_xml()
    {
        if ($this->session->userdata('role') != M_ADMIN)
        {
            AuthTool::unauthorized();
            return;
        }
    
        $data['upload_error'] = '';
        $data['import_status'] = array('data' => '', 'status' => 0);
        $data['export_error'] = '';
        
        $this->load->helper('file');
        $this->load->model('xml_model');
        
        if ($this->input->post('import_flag'))
        {        
            $result = $this->xml_model->do_upload();
            
            if ($result['error'])
            {
                $data['upload_error'] = str_replace('<p>', '<p class="error">', $result['error']);
            }
            else
            {
                $data['import_status'] = $this->xml_model->import_xml($result['data']['full_path']);

                $item = $data['import_status']['data'];
                
                if (is_array($item))
                {
                    $data['import_status']['data'] = '&quot;' . $item['id_miest'] . '&quot;, ' . 
                                                     '&quot;' . $item['typ_miest'] . '&quot;, ' . 
                                                     '&quot;' . $item['kapacita'] . '&quot;, ' . 
                                                     '&quot;' . $item['spec_vyb'] . '&quot;';
                }
            }
        }
        else if ($this->input->post('export_flag'))
        {
            $this->xml_model->export_xml();
        }
        
        AuthTool::load_template('man_xml_view', 'Import/Export tabuľky vo formáte XML', $data);
    }
    
    public function man_classrooms()
    {
        //redirect('search/classroom');
        
        $data = SetupTool::setup_roomsearch();
        $data['results_admin'] = 0;
        $data['introduction_flag'] = 1;
        $data['submit'] = 'management/man_classrooms';
        if (    $data['results'] === 1 && 
                ($query = $this->search_model->search_classroom())  )
        {
            // got some data
            $data['results'] = 0;
            $data['results_admin'] = 1;
            $data['query'] = $query;
        }
        AuthTool::load_admin(array('search_classroom_view', 'man_classrooms_view'), 'Učebne', $data);

    }
    
    public function del_classroom()
    {
        //$data['results_admin'] = 1
        if( AuthTool::check_admin_rights() )
        {
            $checkboxes = $this->input->post('del_cl_chk');
            if(empty($checkboxes))
            {
                ?><script>alert("Označte učenbne na mazanie!");</script><?php
            }
            else
            {
                $this->load->model('classroom_model');
                foreach ($checkboxes as $value) {
                    $this->classroom_model->remove_classroom($value);
                    ?><script>alert("Posluchárna <?php echo $value; ?> vymazaná");</script><?php
                }
            }
        
            $this->man_classrooms();
        }
        
    }
    
    public function add_classroom()
    {
        if( AuthTool::check_admin_rights() )
        {   
            $data = SetupTool::setup_roomedit($is_addition = TRUE);
            if( $data['activate'] === TRUE) 
            {
                $this->load->model('classroom_model');
                if($this->classroom_model->add_classroom($data['values']))
                {
                    ?><script>alert("Učebňa pridaná");</script><?php
                } else
                {
                    ?><script>alert("Učebňa <?php echo $data['values']['id_miest'];?> už existuje!");</script><?php
                }
                
            }
            $data['submit'] = 'management/add_classroom';
            $data['action'] = 'Pridať';
            AuthTool::load_admin('add_classroom_view', 'Pridať učebňu', $data);
        }
    }
    
    public function edit_classroom($id)
    {
        if( AuthTool::check_admin_rights() )
        {   
            $this->load->model('classroom_model');
            $data = SetupTool::setup_roomedit($is_addition = FALSE);
            if( $data['activate'] === TRUE) 
            {
                //$data['values']['id_miest'] = $id;
                //if(empty($this->classroom_model->get_classroom($id))
                $this->classroom_model->edit_classroom($data['values'], $id);
                ?><script>alert("Učebňa zmenená");</script><?php
                $this->man_classrooms();
                return;
            }
            $data['submit'] = "management/edit_classroom/$id";
            $data['action'] = 'Zmeniť';
            $result = $this->classroom_model->get_classroom($id);
            $data['editables'] ['id_miest'] = $id;
            foreach ($result[0] as $key => $value) {
                $data['editables'] [$key] = $value;
                //echo $value->id_miest;
            }
            AuthTool::load_admin('edit_classroom_view', 'Zmeniť učebňu', $data);
        }
    }
    
    public function add_user()
    {
        
        if( !AuthTool::check_admin_rights() )
        {
            return;
        }
        
        $data['results'] = '';
        $data['query'] = '';
        $data['zam_error'] = '';
        
        if ($this->input->post('post_flag'))
        {
            $this->load->library('form_validation');
            
            // form validation   
            $this->form_validation->set_rules('r_cislo', 'rodné číslo', 'trim|required|min_length[11]|max_length[11]');
            $this->form_validation->set_rules('priezvisko', 'priezvisko', 'trim|min_length[1]|max_length[50]');
            $this->form_validation->set_rules('meno', 'meno', 'trim|required|min_length[1]|max_length[50]');
            $this->form_validation->set_rules('titul', 'titul', 'trim|min_length[1]|max_length[50]');
            $this->form_validation->set_rules('zobraz_meno', 'zobrazovacie meno', 'trim|required|min_length[1]|max_length[100]');
            $this->form_validation->set_rules('username', 'prihlasovacie meno', 'trim|required|min_length[1]|max_length[20]');
            $this->form_validation->set_rules('heslo', 'heslo', 'trim|required|min_length[1]|max_length[30]');
            $this->form_validation->set_rules('prac_pomer', 'pracovný pomer', 'trim|required|min_length[1]|max_length[50]');
            
            if($this->form_validation->run() == TRUE)
            {
                $this->load->model('user_model');

                // check for free date
                if ($this->user_model->check_zam_free())
                {
                    // validation OK
                    if ($query = $this->user_model->add_user())
                    {
                        // got some data
                        $data['results'] = 1;
                        $data['query'] = $query;
                    }
                }
                else
                {
                    $data['zam_error'] = 2;
                }
                
            }
        }
        
        AuthTool::load_template('add_user_view', 'Pridať zamestnanec', $data);
        
    }
    
    public function del_user()
    {
        $checkboxes = $this->input->post('del_cl_chk');
        if(empty($checkboxes))
        {
            ?><script>alert("Označte učenbne na mazanie!");</script><?php
        }
        else
        {
            $this->load->model('user_model');
            foreach ($checkboxes as $value) {
                $this->user_model->remove_user($value);
                ?><script>alert("Zamestnanec <?php echo $value; ?> vymazaná");</script><?php
            }
        }
        
        $this->index();
    }
    
    public function edit_user()
    {
        
    }
    
}

?>
