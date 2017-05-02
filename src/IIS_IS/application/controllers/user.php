<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');
require_once 'includes/authtool.php';

/**
 * Description of Site
 * Controller for controlling user related funcionality 
 * ( it means adding reservations for now )
 * @author xmolna02
 */
class User extends CI_Controller {
    
    public function add_reservation()
    {
        if ($this->session->userdata('role') == M_UNAUTH)
        {
            AuthTool::unauthorized();
            return;
        }
        
        $data['results'] = '';
        $data['query'] = '';
        $data['datum_error'] = '';
        
        $this->load->helper('date');
        
        // ucebna dropdown
        $this->load->model('search_model');
    
        $ucebna_dd = array('' => '');
        $query = $this->search_model->get_ucebne();

        foreach ($query->result() as $row)
        {
            $ucebna_dd[$row->id_miest] = $row->id_miest;
        }
        
        $data['ucebna_dd'] = $ucebna_dd;
        
        // post
        if ($this->input->post('post_flag'))
        {
            $this->load->library('form_validation');
            
            // form validation   
            $this->form_validation->set_rules('ucebna', 'učebňa', 'trim|required|min_length[4]|max_length[4]');
            $this->form_validation->set_rules('typ', 'typ udalosti', 'trim|min_length[1]|max_length[100]');
            $this->form_validation->set_rules('datum1', 'termín od', 'trim|required|min_length[1]|max_length[20]');
            $this->form_validation->set_rules('datum2', 'termín do', 'trim|required|min_length[1]|max_length[20]');

            if($this->form_validation->run() == TRUE)
            {
                $time1 = strtotime(sk_to_date($this->input->post('datum1')));
                $time2 = strtotime(sk_to_date($this->input->post('datum2')));
            
                if ($time1 >= $time2)
                {
                    $data['datum_error'] = 1;
                }
                else
                {
                    $this->load->model('user_model');
                    
                    // check for free date
                    if ($this->user_model->check_free_reservation())
                    {
                        // validation OK
                        if ($query = $this->user_model->add_reservation())
                        {
                            // got some data
                            $data['results'] = 1;
                        	$data['query'] = $query;
                        }
                    }
                    else
                    {
                        $data['datum_error'] = 2;
                    }
                }
            }
        }
        
        AuthTool::load_template('add_reser_view', 'Pridať rezerváciu', $data);
    }
}

/* End of file user.php */
/* Location: ./application/controllers/user.php */