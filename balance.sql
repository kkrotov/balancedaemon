--
-- PostgreSQL database dump
--

-- Dumped from database version 9.5.19
-- Dumped by pg_dump version 9.5.19

SET statement_timeout = 0;
SET lock_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SELECT pg_catalog.set_config('search_path', '', false);
SET check_function_bodies = false;
SET xmloption = content;
SET client_min_messages = warning;
SET row_security = off;

--
-- Name: plpgsql; Type: EXTENSION; Schema: -; Owner: 
--

CREATE EXTENSION IF NOT EXISTS plpgsql WITH SCHEMA pg_catalog;


--
-- Name: EXTENSION plpgsql; Type: COMMENT; Schema: -; Owner: 
--

COMMENT ON EXTENSION plpgsql IS 'PL/pgSQL procedural language';


--
-- Name: decrement_balance(text, money); Type: FUNCTION; Schema: public; Owner: kkrotov
--

CREATE FUNCTION public.decrement_balance(name text, val money) RETURNS money
    LANGUAGE plpgsql
    AS $$begin
	update public.userbalance set amount = amount-val where username=name;
	return val;
end;$$;


ALTER FUNCTION public.decrement_balance(name text, val money) OWNER TO kkrotov;

--
-- Name: increment_balance(text, money); Type: FUNCTION; Schema: public; Owner: kkrotov
--

CREATE FUNCTION public.increment_balance(name text, val money) RETURNS money
    LANGUAGE plpgsql
    AS $$begin
	update public.userbalance set amount = amount+val where username=name;
	return val;
end;$$;


ALTER FUNCTION public.increment_balance(name text, val money) OWNER TO kkrotov;

--
-- Name: log_balance_changes(); Type: FUNCTION; Schema: public; Owner: kkrotov
--

CREATE FUNCTION public.log_balance_changes() RETURNS trigger
    LANGUAGE plpgsql
    AS $$
DECLARE
    optype boolean;
    amount_change money;
BEGIN
   IF NEW.amount < OLD.amount THEN
        optype := false;
        amount_change := OLD.amount - NEW.amount;
   ELSE
        optype := true;
        amount_change := NEW.amount - OLD.amount;
   END IF;

   INSERT INTO operationlog(userid,datetime,operationtype,amount)
   VALUES (NEW.id, now(), optype, amount_change);
 
   RETURN NEW;
END;
$$;


ALTER FUNCTION public.log_balance_changes() OWNER TO kkrotov;

SET default_tablespace = '';

SET default_with_oids = false;

--
-- Name: operationlog; Type: TABLE; Schema: public; Owner: kkrotov
--

CREATE TABLE public.operationlog (
    userid integer,
    datetime timestamp with time zone DEFAULT now(),
    operationtype boolean,
    amount money
);


ALTER TABLE public.operationlog OWNER TO kkrotov;

--
-- Name: userbalance; Type: TABLE; Schema: public; Owner: kkrotov
--

CREATE TABLE public.userbalance (
    id integer NOT NULL,
    username character varying(80),
    amount money,
    CONSTRAINT amount_nonnegative CHECK ((amount >= (0)::money))
);


ALTER TABLE public.userbalance OWNER TO kkrotov;

--
-- Name: userbalance_id_seq; Type: SEQUENCE; Schema: public; Owner: kkrotov
--

CREATE SEQUENCE public.userbalance_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public.userbalance_id_seq OWNER TO kkrotov;

--
-- Name: userbalance_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: kkrotov
--

ALTER SEQUENCE public.userbalance_id_seq OWNED BY public.userbalance.id;


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: kkrotov
--

ALTER TABLE ONLY public.userbalance ALTER COLUMN id SET DEFAULT nextval('public.userbalance_id_seq'::regclass);


--
-- Data for Name: operationlog; Type: TABLE DATA; Schema: public; Owner: kkrotov
--

COPY public.operationlog (userid, datetime, operationtype, amount) FROM stdin;
\.


--
-- Data for Name: userbalance; Type: TABLE DATA; Schema: public; Owner: kkrotov
--

COPY public.userbalance (id, username, amount) FROM stdin;
3	smirnov	0.00 руб
1	ivanov	0.00 руб
2	petrov	0.00 руб
\.


--
-- Name: userbalance_id_seq; Type: SEQUENCE SET; Schema: public; Owner: kkrotov
--

SELECT pg_catalog.setval('public.userbalance_id_seq', 3, true);


--
-- Name: id; Type: CONSTRAINT; Schema: public; Owner: kkrotov
--

ALTER TABLE ONLY public.userbalance
    ADD CONSTRAINT id PRIMARY KEY (id);


--
-- Name: t_balance_update; Type: TRIGGER; Schema: public; Owner: kkrotov
--

CREATE TRIGGER t_balance_update AFTER INSERT OR DELETE OR UPDATE ON public.userbalance FOR EACH ROW EXECUTE PROCEDURE public.log_balance_changes();


--
-- Name: userid; Type: FK CONSTRAINT; Schema: public; Owner: kkrotov
--

ALTER TABLE ONLY public.operationlog
    ADD CONSTRAINT userid FOREIGN KEY (userid) REFERENCES public.userbalance(id);


--
-- Name: SCHEMA public; Type: ACL; Schema: -; Owner: postgres
--

REVOKE ALL ON SCHEMA public FROM PUBLIC;
REVOKE ALL ON SCHEMA public FROM postgres;
GRANT ALL ON SCHEMA public TO postgres;
GRANT ALL ON SCHEMA public TO PUBLIC;


--
-- PostgreSQL database dump complete
--

