<?php
class Benchmark
{
    protected $iteration;
    protected $closures;
    protected $times;
    protected $args;

    public function __construct($iteration = 1000)
    {
        $this->iteration = $iteration;
        $this->closures = array();
        $this->args = array(0 => array());
        $this->times = array();
    }

    public function registerArg()
    {
        $this->registerArgs(func_get_args());

        return $this;
    }

    public function registerArgs(Array $args)
    {
        $this->args[0] += $args;

        return $this;
    }

    public function add(Closure $f, $name, Array $args = array())
    {
        $this->closures[$name] = $f;
        if ($args) {
            $this->args[$name] = $args;
        }

        return $this;
    }

    public function execute()
    {
        foreach ($this->closures as $name => $f) {
            $args = isset($this->args[$name]) ? $this->args[$name] : $this->args[0];
            $start = microtime(TRUE);
            for ($i = 0; $i < $this->iteration; $i++) {
                call_user_func_array($f, $args);
            }
            $this->times[$name] = microtime(TRUE) - $start;
        }

        return $this;
    }

    public function report()
    {
        define('CLI_EOL', PHP_SAPI == 'cli' ? PHP_EOL : "<br />\n");

        if ($this->times) {
            $min = min($this->times);
            $max = max($this->times);
            foreach ($this->times as $n => $t) {
                echo str_repeat('-', 60), CLI_EOL;
                if ($t == $min) {
                    printf('%s : %f' . CLI_EOL, $n, $t);
                } else {
                    printf('%s : %f (~ %d)' . CLI_EOL, $n, $t, $t / $min);
                }
            }
            if ($this->times > 1) {
                echo str_repeat('=', 60), CLI_EOL;
                printf('Best  : %s' . PHP_EOL, array_search($min, $this->times));
                printf('Worst : %s' . PHP_EOL, array_search($max, $this->times));
                //printf("Efficiency ratio : ~ %d" . PHP_EOL, $max / $min);
            }
        }

        return $this;
    }
}
