<?php

/**
 * Description of AuthTool
 * Class defining functions for checking authenticated users
 * This class is not intended for instantiation
 * @author Mészáros Tamás
 */
class AuthTool {
    
    // no instantiation is permitted
    private function __construct() {
        ;
    }
    
    /**
     * Function checks if the user has administation rights
     * @return bool
     */
    public function check_admin_rights()
    {
        if ($this->session->userdata('role') == M_ADMIN)
        {
            return TRUE;
        } else
        {
            AuthTool::unauthorized();
            return FALSE;
        }
    }
    
    /**
     * Loads a page checking for admin privileges
     * @param String $view_name
     * @param String $title
     * @param array $data 
     */
    public function load_admin($view_name, $title, $data)
    {
        if ($this->session->userdata('role') == M_ADMIN)
        {
            AuthTool::load_template($view_name, $title, $data);
        }
        else
        {
            AuthTool::unauthorized();
        }
    }
    
    /**
     * loads a constructed a page with the given content (view) 
     * @param String $content The name of the view with content
     * @param String $title The page title
     * @param array $param Data forwarded to the page
     */
    public function load_template($content, $title, $param)
    {
        $data = array();
        
        foreach ($param as $k => $v)
        {
            $data[$k] = $v;
        }
        
        $data['main_content'] = $content;
        $data['username'] = $this->session->userdata('username');
        $data['fname'] = $this->session->userdata('fname');
        $data['role'] = $this->session->userdata('role');
        $data['page_title'] = $title;
        $this->load->view('includes/template', $data);
    }
    
    /**
     * Shows the unathorized user page
     */
    public function unauthorized()
    {
        $data = array();
        AuthTool::load_template('unauth_view', 'Nedostatočné práva', $data);
    }
}

/* End of file authtool.php */
/* Location: ./application/controllers/includes/authtool.php */