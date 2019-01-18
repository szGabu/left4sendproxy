/**
 * vim: set ts=4 :
 * =============================================================================
 * SendVar Proxy Manager
 * Copyright (C) 2011-2019 Afronanny & AlliedModders community.  All rights reserved.
 * =============================================================================
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License, version 3.0, as published by the
 * Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * As a special exception, AlliedModders LLC gives you permission to link the
 * code of this program (as well as its derivative works) to "Half-Life 2," the
 * "Source Engine," the "SourcePawn JIT," and any Game MODs that run on software
 * by the Valve Corporation.  You must obey the GNU General Public License in
 * all respects for all other code used.  Additionally, AlliedModders LLC grants
 * this exception to all derivative works.  AlliedModders LLC defines further
 * exceptions, found in LICENSE.txt (as of this writing, version JULY-31-2007),
 * or <http://www.sourcemod.net/license.php>.
 *
 * Version: $Id$
 */

#include "interface.h"

SH_DECL_HOOK0_void(IExtensionInterface, OnExtensionUnload, SH_NOATTRIB, false);

void Hook_OnExtensionUnload()
{
	IExtensionInterface * pExtAPI = META_IFACEPTR(IExtensionInterface);
	for (int i = 0; i < g_Hooks.Count(); i++)
	{
		//remove all listeners for this extension
		if (g_Hooks[i].vListeners.Count())
			for (int j = 0; j < g_Hooks[i].vListeners.Count(); j++)
			{
				ListenerCallbackInfo info = g_Hooks[i].vListeners[j];
				if (info.m_pExtAPI == pExtAPI)
					g_Hooks[i].vListeners.Remove(j);
			}
		//remove hook for this extension
		if (g_HooksGamerules[i].pExtensionAPI == pExtAPI)
			g_SendProxyManager.UnhookProxy(i);
	}
	for (int i = 0; i < g_HooksGamerules.Count(); i++)
	{
		//remove all listeners for this extension
		if (g_HooksGamerules[i].vListeners.Count())
			for (int j = 0; j < g_HooksGamerules[i].vListeners.Count(); j++)
			{
				ListenerCallbackInfo info = g_HooksGamerules[i].vListeners[j];
				if (info.m_pExtAPI == pExtAPI)
					g_HooksGamerules[i].vListeners.Remove(j);
			}
		//remove hook for this extension
		if (g_HooksGamerules[i].pExtensionAPI == pExtAPI)
			g_SendProxyManager.UnhookProxyGamerules(i);
	}
	SH_REMOVE_HOOK(IExtensionInterface, OnExtensionUnload, pExtAPI, SH_STATIC(Hook_OnExtensionUnload), false);
	RETURN_META(MRES_IGNORED);
}

void HookExtensionUnload(IExtension * pExt)
{
	if (!pExt)
		return;
	
	bool bHookedAlready = false;
	for (int i = 0; i < g_Hooks.Count(); i++)
		if (g_Hooks[i].pExtensionAPI == pExt->GetAPI())
		{
			bHookedAlready = true;
			break;
		}
		else if (g_Hooks[i].vListeners.Count())
			for (int j = 0; j < g_Hooks[i].vListeners.Count(); j++)
			{
				ListenerCallbackInfo info = g_Hooks[i].vListeners[j];
				if (info.m_pExtAPI == pExt->GetAPI())
				{
					bHookedAlready = true;
					break;
				}
			}
	if (!bHookedAlready)
		for (int i = 0; i < g_HooksGamerules.Count(); i++)
			if (g_HooksGamerules[i].pExtensionAPI == pExt->GetAPI())
			{
				bHookedAlready = true;
				break;
			}
			else if (g_HooksGamerules[i].vListeners.Count())
				for (int j = 0; j < g_HooksGamerules[i].vListeners.Count(); j++)
				{
					ListenerCallbackInfo info = g_HooksGamerules[i].vListeners[j];
					if (info.m_pExtAPI == pExt->GetAPI())
					{
						bHookedAlready = true;
						break;
					}
				}
	if (!bHookedAlready) //Hook only if needed!
		SH_ADD_HOOK(IExtensionInterface, OnExtensionUnload, pExt->GetAPI(), SH_STATIC(Hook_OnExtensionUnload), false);
}

void UnhookExtensionUnload(IExtension * pExt)
{
	if (!pExt)
		return;
	
	bool bHaveHooks = false;
	for (int i = 0; i < g_Hooks.Count(); i++)
		if (g_Hooks[i].pExtensionAPI == pExt->GetAPI())
		{
			bHaveHooks = true;
			break;
		}
		else if (g_Hooks[i].vListeners.Count())
			for (int j = 0; j < g_Hooks[i].vListeners.Count(); j++)
			{
				ListenerCallbackInfo info = g_Hooks[i].vListeners[j];
				if (info.m_pExtAPI == pExt->GetAPI())
				{
					bHaveHooks = true;
					break;
				}
			}
	if (!bHaveHooks)
		for (int i = 0; i < g_HooksGamerules.Count(); i++)
			if (g_HooksGamerules[i].pExtensionAPI == pExt->GetAPI())
			{
				bHaveHooks = true;
				break;
			}
			else if (g_HooksGamerules[i].vListeners.Count())
				for (int j = 0; j < g_HooksGamerules[i].vListeners.Count(); j++)
				{
					ListenerCallbackInfo info = g_HooksGamerules[i].vListeners[j];
					if (info.m_pExtAPI == pExt->GetAPI())
					{
						bHaveHooks = true;
						break;
					}
				}
	
	if (!bHaveHooks) //so, if there are active hooks, we shouldn't remove hook!
		SH_REMOVE_HOOK(IExtensionInterface, OnExtensionUnload, pExt->GetAPI(), SH_STATIC(Hook_OnExtensionUnload), false);
}

void CallListenersForHookID(int iHookID)
{
	SendPropHook Info = g_Hooks[iHookID];
	for (int i = 0; i < Info.vListeners.Count(); i++)
	{
		ListenerCallbackInfo sInfo = Info.vListeners[i];
		sInfo.m_pCallBack->OnEntityPropHookRemoved(gameents->EdictToBaseEntity(Info.pEnt), Info.pVar, Info.PropType, Info.iCallbackType, Info.pCallback);
	}
}

void CallListenersForHookIDGamerules(int iHookID)
{
	SendPropHookGamerules Info = g_HooksGamerules[iHookID];
	for (int i = 0; i < Info.vListeners.Count(); i++)
	{
		ListenerCallbackInfo sInfo = Info.vListeners[i];
		sInfo.m_pCallBack->OnGamerulesPropHookRemoved(Info.pVar, Info.PropType, Info.iCallbackType, Info.pCallback);
	}
}

//interface

const char * SendProxyManagerInterfaceImpl::GetInterfaceName() { return SMINTERFACE_SENDPROXY_NAME; }
unsigned int SendProxyManagerInterfaceImpl::GetInterfaceVersion() { return SMINTERFACE_SENDPROXY_VERSION; }

bool SendProxyManagerInterfaceImpl::HookProxy(IExtension * pExt, SendProp * pProp, CBaseEntity * pEntity, PropType iType, CallBackType iCallbackType, void * pCallback)
{
	if (!pEntity)
		return false;
	
	edict_t * pEdict = gameents->BaseEntityToEdict(pEntity);
	if (!pEdict || pEdict->IsFree())
		return false;

	if (!IsPropValid(pProp, iType))
		return false;
	
	SendPropHook hook;
	hook.objectID = gamehelpers->IndexOfEdict(pEdict);
	hook.pCallback = pCallback;
	hook.iCallbackType = iCallbackType;
	hook.PropType = iType;
	hook.pEnt = pEdict;
	hook.pVar = pProp;
	hook.pExtensionAPI = pExt->GetAPI();
	bool bHookedAlready = false;
	for (int i = 0; i < g_Hooks.Count(); i++)
	{
		if (g_Hooks[i].pVar == pProp)
		{
			hook.pRealProxy = g_Hooks[i].pRealProxy;
			bHookedAlready = true;
			break;
		}
	}
	if (!bHookedAlready)
		hook.pRealProxy = pProp->GetProxyFn();
	HookExtensionUnload(pExt);
	if (g_SendProxyManager.AddHookToList(hook))
	{
		if (!bHookedAlready)
			pProp->SetProxyFn(GlobalProxy);
	}
	else
	{
		UnhookExtensionUnload(pExt);
		return false;
	}
	return true;
}

bool SendProxyManagerInterfaceImpl::HookProxy(IExtension * pExt, const char * pProp, CBaseEntity * pEntity, PropType iType, CallBackType iCallbackType, void * pCallback)
{
	if (!pProp || !*pProp)
		return false;
	if (!pEntity)
		return false;
	ServerClass * sc = ((IServerUnknown *)pEntity)->GetNetworkable()->GetServerClass();
	if (!sc)
		return false; //we don't use exceptions, bad extensions may do not handle this and server will crashed, just return false
	sm_sendprop_info_t info;
	gamehelpers->FindSendPropInfo(sc->GetName(), pProp, &info);
	SendProp * pSendProp = info.prop;
	if (pSendProp)
		return HookProxy(pExt, pSendProp, pEntity, iType, iCallbackType, pCallback);
	return false;
}

bool SendProxyManagerInterfaceImpl::HookProxyGamerules(IExtension * pExt, SendProp * pProp, PropType iType, CallBackType iCallbackType, void * pCallback)
{
	if (!IsPropValid(pProp, iType))
		return false;
	
	SendPropHookGamerules hook;
	hook.pCallback = pCallback;
	hook.iCallbackType = iCallbackType;
	bool bHookedAlready = false;
	for (int i = 0; i < g_HooksGamerules.Count(); i++)
	{
		if (g_HooksGamerules[i].pVar == pProp)
		{
			hook.pRealProxy = g_HooksGamerules[i].pRealProxy;
			bHookedAlready = true;
			break;
		}
	}
	if (!bHookedAlready)
		hook.pRealProxy = pProp->GetProxyFn();
	hook.PropType = iType;
	hook.pVar = pProp;
	hook.pExtensionAPI = pExt->GetAPI();
	sm_sendprop_info_t info;
	gamehelpers->FindSendPropInfo(g_szGameRulesProxy, pProp->GetName(), &info);
	hook.Offset = info.actual_offset;
	
	//if this prop has been hooked already, don't set the proxy again
	HookExtensionUnload(pExt);
	if (bHookedAlready)
	{
		if (g_SendProxyManager.AddHookToListGamerules(hook))
			return true;
		UnhookExtensionUnload(pExt);
		return false;
	}
	if (g_SendProxyManager.AddHookToListGamerules(hook))
	{
		pProp->SetProxyFn(GlobalProxyGamerules);
		return true;
	}
	UnhookExtensionUnload(pExt);
	return false;
}

bool SendProxyManagerInterfaceImpl::HookProxyGamerules(IExtension * pExt, const char * pProp, PropType iType, CallBackType iCallbackType, void * pCallback)
{
	if (!pProp || !*pProp)
		return false;
	sm_sendprop_info_t info;
	gamehelpers->FindSendPropInfo(g_szGameRulesProxy, pProp, &info);
	SendProp * pSendProp = info.prop;
	if (pSendProp)
		return HookProxyGamerules(pExt, pSendProp, iType, iCallbackType, pCallback);
	return false;
}

bool SendProxyManagerInterfaceImpl::UnhookProxy(IExtension * pExt, SendProp * pProp, CBaseEntity * pEntity, CallBackType iCallbackType, void * pCallback)
{
	const char * pPropName = pProp->GetName();
	return UnhookProxy(pExt, pPropName, pEntity, iCallbackType, pCallback);
}

bool SendProxyManagerInterfaceImpl::UnhookProxy(IExtension * pExt, const char * pProp, CBaseEntity * pEntity, CallBackType iCallbackType, void * pCallback)
{
	if (!pProp || !*pProp)
		return false;
	edict_t * pEdict = gameents->BaseEntityToEdict(pEntity);
	for (int i = 0; i < g_Hooks.Count(); i++)
		if (g_Hooks[i].pExtensionAPI == pExt->GetAPI() /*Allow to extension remove only its hooks*/ && pEdict == g_Hooks[i].pEnt && g_Hooks[i].iCallbackType == iCallbackType && !strcmp(g_Hooks[i].pVar->GetName(), pProp) && pCallback == g_Hooks[i].pCallback)
		{
			g_SendProxyManager.UnhookProxy(i);
			UnhookExtensionUnload(pExt);
			return true;
		}
	return false;
}

bool SendProxyManagerInterfaceImpl::UnhookProxyGamerules(IExtension * pExt, SendProp * pProp, CallBackType iCallbackType, void * pCallback)
{
	const char * pPropName = pProp->GetName();
	return UnhookProxyGamerules(pExt, pPropName, iCallbackType, pCallback);
}

bool SendProxyManagerInterfaceImpl::UnhookProxyGamerules(IExtension * pExt, const char * pProp, CallBackType iCallbackType, void * pCallback)
{
	if (!pProp || !*pProp)
		return false;
	for (int i = 0; i < g_HooksGamerules.Count(); i++)
		if (g_HooksGamerules[i].pExtensionAPI == pExt->GetAPI() /*Allow to extension remove only its hooks*/ && g_HooksGamerules[i].iCallbackType == iCallbackType && !strcmp(g_HooksGamerules[i].pVar->GetName(), pProp) && pCallback == g_HooksGamerules[i].pCallback)
		{
			g_SendProxyManager.UnhookProxyGamerules(i);
			UnhookExtensionUnload(pExt);
			return true;
		}
	return false;
}

bool SendProxyManagerInterfaceImpl::AddUnhookListener(IExtension * pExt, SendProp * pProp, CBaseEntity * pEntity, CallBackType iCallbackType, void * pCallback, ISendProxyUnhookListener * pListener)
{
	const char * pPropName = pProp->GetName();
	return AddUnhookListener(pExt, pPropName, pEntity, iCallbackType, pCallback, pListener);
}

bool SendProxyManagerInterfaceImpl::AddUnhookListener(IExtension * pExt, const char * pProp, CBaseEntity * pEntity, CallBackType iCallbackType, void * pCallback, ISendProxyUnhookListener * pListener)
{
	if (!pProp || !*pProp)
		return false;
	edict_t * pEdict = gameents->BaseEntityToEdict(pEntity);
	for (int i = 0; i < g_Hooks.Count(); i++)
		if (pEdict == g_Hooks[i].pEnt && g_Hooks[i].iCallbackType == iCallbackType && !strcmp(g_Hooks[i].pVar->GetName(), pProp) && pCallback == g_Hooks[i].pCallback)
		{
			ListenerCallbackInfo info;
			info.m_pExt = pExt;
			info.m_pExtAPI = pExt->GetAPI();
			info.m_pCallBack = pListener;
			HookExtensionUnload(pExt);
			g_Hooks[i].vListeners.AddToTail(info);
			return true;
		}
	return false;
}

bool SendProxyManagerInterfaceImpl::AddUnhookListenerGamerules(IExtension * pExt, SendProp * pProp, CallBackType iCallbackType, void * pCallback, ISendProxyUnhookListener * pListener)
{
	const char * pPropName = pProp->GetName();
	return AddUnhookListenerGamerules(pExt, pPropName, iCallbackType, pCallback, pListener);
}

bool SendProxyManagerInterfaceImpl::AddUnhookListenerGamerules(IExtension * pExt, const char * pProp, CallBackType iCallbackType, void * pCallback, ISendProxyUnhookListener * pListener)
{
	if (!pProp || !*pProp)
		return false;
	for (int i = 0; i < g_HooksGamerules.Count(); i++)
		if (g_HooksGamerules[i].iCallbackType == iCallbackType && !strcmp(g_HooksGamerules[i].pVar->GetName(), pProp) && pCallback == g_HooksGamerules[i].pCallback)
		{
			ListenerCallbackInfo info;
			info.m_pExt = pExt;
			info.m_pExtAPI = pExt->GetAPI();
			info.m_pCallBack = pListener;
			HookExtensionUnload(pExt);
			g_HooksGamerules[i].vListeners.AddToTail(info);
			return true;
		}
	return false;
}

bool SendProxyManagerInterfaceImpl::RemoveUnhookListener(IExtension * pExt, SendProp * pProp, CBaseEntity * pEntity, CallBackType iCallbackType, void * pCallback, ISendProxyUnhookListener * pListener)
{
	const char * pPropName = pProp->GetName();
	return RemoveUnhookListener(pExt, pPropName, pEntity, iCallbackType, pCallback, pListener);
}

bool SendProxyManagerInterfaceImpl::RemoveUnhookListener(IExtension * pExt, const char * pProp, CBaseEntity * pEntity, CallBackType iCallbackType, void * pCallback, ISendProxyUnhookListener * pListener)
{
	if (!pProp || !*pProp)
		return false;
	
	edict_t * pEdict = gameents->BaseEntityToEdict(pEntity);
	for (int i = 0; i < g_Hooks.Count(); i++)
		if (g_Hooks[i].pEnt == pEdict && g_Hooks[i].iCallbackType == iCallbackType && !strcmp(g_Hooks[i].pVar->GetName(), pProp) && pCallback == g_Hooks[i].pCallback)
		{
			for (int j = 0; j < g_Hooks[i].vListeners.Count(); j++)
			{
				ListenerCallbackInfo info = g_Hooks[i].vListeners[j];
				if (info.m_pExt == pExt && info.m_pCallBack == pListener)
				{
					g_Hooks[i].vListeners.Remove(j);
					UnhookExtensionUnload(pExt);
					return true;
				}
			}
		}
	return false;
}

bool SendProxyManagerInterfaceImpl::RemoveUnhookListenerGamerules(IExtension * pExt, SendProp * pProp, CallBackType iCallbackType, void * pCallback, ISendProxyUnhookListener * pListener)
{
	const char * pPropName = pProp->GetName();
	return RemoveUnhookListenerGamerules(pExt, pPropName, iCallbackType, pCallback, pListener);
}

bool SendProxyManagerInterfaceImpl::RemoveUnhookListenerGamerules(IExtension * pExt, const char * pProp, CallBackType iCallbackType, void * pCallback, ISendProxyUnhookListener * pListener)
{
	if (!pProp || !*pProp)
		return false;
	
	for (int i = 0; i < g_HooksGamerules.Count(); i++)
		if (g_HooksGamerules[i].iCallbackType == iCallbackType && !strcmp(g_HooksGamerules[i].pVar->GetName(), pProp) && pCallback == g_HooksGamerules[i].pCallback)
		{
			for (int j = 0; j < g_HooksGamerules[i].vListeners.Count(); j++)
			{
				ListenerCallbackInfo info = g_HooksGamerules[i].vListeners[j];
				if (info.m_pExt == pExt && info.m_pCallBack == pListener)
				{
					g_HooksGamerules[i].vListeners.Remove(j);
					UnhookExtensionUnload(pExt);
					return true;
				}
			}
		}
	return false;
}

//same for the arrays

bool SendProxyManagerInterfaceImpl::HookProxyArray(IExtension * pExt, SendProp * pProp, CBaseEntity * pEntity, PropType iType, int iElement, CallBackType iCallbackType, void * pCallback)
{
	if (!pEntity)
		return false;
	
	edict_t * pEdict = gameents->BaseEntityToEdict(pEntity);
	if (!pEdict || pEdict->IsFree())
		return false;

	SendTable * pSendTable = pProp->GetDataTable();
	if (!pSendTable)
		return false;
	
	SendProp * pPropElem = pSendTable->GetProp(iElement);
	if (!pPropElem)
		return false;
	
	if (!IsPropValid(pPropElem, iType))
		return false;
	
	SendPropHook hook;
	hook.objectID = gamehelpers->IndexOfEdict(pEdict);
	hook.pCallback = pCallback;
	hook.iCallbackType = iCallbackType;
	hook.PropType = iType;
	hook.pEnt = pEdict;
	hook.pVar = pPropElem;
	hook.pExtensionAPI = pExt->GetAPI();
	hook.Element = iElement;
	bool bHookedAlready = false;
	for (int i = 0; i < g_Hooks.Count(); i++)
	{
		if (g_Hooks[i].pVar == pPropElem)
		{
			hook.pRealProxy = g_Hooks[i].pRealProxy;
			bHookedAlready = true;
			break;
		}
	}
	if (!bHookedAlready)
		hook.pRealProxy = pPropElem->GetProxyFn();
	HookExtensionUnload(pExt);
	if (g_SendProxyManager.AddHookToList(hook))
	{
		if (!bHookedAlready)
			pPropElem->SetProxyFn(GlobalProxy);
	}
	else
	{
		UnhookExtensionUnload(pExt);
		return false;
	}
	return true;
}

bool SendProxyManagerInterfaceImpl::HookProxyArray(IExtension * pExt, const char * pProp, CBaseEntity * pEntity, PropType iType, int iElement, CallBackType iCallbackType, void * pCallback)
{
	if (!pProp || !*pProp)
		return false;
	if (!pEntity)
		return false;
	ServerClass * sc = ((IServerUnknown *)pEntity)->GetNetworkable()->GetServerClass();
	if (!sc)
		return false; //we don't use exceptions, bad extensions may do not handle this and server will crashed, just return false
	sm_sendprop_info_t info;
	gamehelpers->FindSendPropInfo(sc->GetName(), pProp, &info);
	SendProp * pSendProp = info.prop;
	if (pSendProp)
		return HookProxyArray(pExt, pSendProp, pEntity, iType, iElement, iCallbackType, pCallback);
	return false;
}

bool SendProxyManagerInterfaceImpl::HookProxyArrayGamerules(IExtension * pExt, SendProp * pProp, PropType iType, int iElement, CallBackType iCallbackType, void * pCallback)
{
	SendTable * pSendTable = pProp->GetDataTable();
	if (!pSendTable)
		return false;
	
	SendProp * pPropElem = pSendTable->GetProp(iElement);
	if (!pPropElem)
		return false;
	
	if (!IsPropValid(pPropElem, iType))
		return false;
	
	SendPropHookGamerules hook;
	hook.pCallback = pCallback;
	hook.iCallbackType = iCallbackType;
	bool bHookedAlready = false;
	for (int i = 0; i < g_HooksGamerules.Count(); i++)
	{
		if (g_HooksGamerules[i].pVar == pProp)
		{
			hook.pRealProxy = g_HooksGamerules[i].pRealProxy;
			bHookedAlready = true;
			break;
		}
	}
	if (!bHookedAlready)
		hook.pRealProxy = pProp->GetProxyFn();
	hook.PropType = iType;
	hook.pVar = pProp;
	hook.pExtensionAPI = pExt->GetAPI();
	sm_sendprop_info_t info;
	gamehelpers->FindSendPropInfo(g_szGameRulesProxy, pProp->GetName(), &info);
	hook.Offset = info.actual_offset;
	hook.Element = iElement;
	
	//if this prop has been hooked already, don't set the proxy again
	HookExtensionUnload(pExt);
	if (bHookedAlready)
	{
		if (g_SendProxyManager.AddHookToListGamerules(hook))
			return true;
		UnhookExtensionUnload(pExt);
		return false;
	}
	if (g_SendProxyManager.AddHookToListGamerules(hook))
	{
		pProp->SetProxyFn(GlobalProxyGamerules);
		return true;
	}
	UnhookExtensionUnload(pExt);
	return false;
}

bool SendProxyManagerInterfaceImpl::HookProxyArrayGamerules(IExtension * pExt, const char * pProp, PropType iType, int iElement, CallBackType iCallbackType, void * pCallback)
{
	if (!pProp || !*pProp)
		return false;
	sm_sendprop_info_t info;
	gamehelpers->FindSendPropInfo(g_szGameRulesProxy, pProp, &info);
	SendProp * pSendProp = info.prop;
	if (pSendProp)
		return HookProxyArrayGamerules(pExt, pSendProp, iType, iElement, iCallbackType, pCallback);
	return false;
}

bool SendProxyManagerInterfaceImpl::UnhookProxyArray(IExtension * pExt, SendProp * pProp, CBaseEntity * pEntity, int iElement, CallBackType iCallbackType, void * pCallback)
{
	const char * pPropName = pProp->GetName();
	return UnhookProxyArray(pExt, pPropName, pEntity, iElement, iCallbackType, pCallback);
}

bool SendProxyManagerInterfaceImpl::UnhookProxyArray(IExtension * pExt, const char * pProp, CBaseEntity * pEntity, int iElement, CallBackType iCallbackType, void * pCallback)
{
	if (!pProp || !*pProp)
		return false;
	edict_t * pEdict = gameents->BaseEntityToEdict(pEntity);
	for (int i = 0; i < g_Hooks.Count(); i++)
		if (g_Hooks[i].pExtensionAPI == pExt->GetAPI() /*Allow to extension remove only its hooks*/ && g_Hooks[i].Element == iElement && pEdict == g_Hooks[i].pEnt && g_Hooks[i].iCallbackType == iCallbackType && !strcmp(g_Hooks[i].pVar->GetName(), pProp) && pCallback == g_Hooks[i].pCallback)
		{
			g_SendProxyManager.UnhookProxy(i);
			UnhookExtensionUnload(pExt);
			return true;
		}
	return false;
}

bool SendProxyManagerInterfaceImpl::UnhookProxyArrayGamerules(IExtension * pExt, SendProp * pProp, int iElement, CallBackType iCallbackType, void * pCallback)
{
	const char * pPropName = pProp->GetName();
	return UnhookProxyArrayGamerules(pExt, pPropName, iElement, iCallbackType, pCallback);
}

bool SendProxyManagerInterfaceImpl::UnhookProxyArrayGamerules(IExtension * pExt, const char * pProp, int iElement, CallBackType iCallbackType, void * pCallback)
{
	if (!pProp || !*pProp)
		return false;
	for (int i = 0; i < g_HooksGamerules.Count(); i++)
		if (g_HooksGamerules[i].pExtensionAPI == pExt->GetAPI() /*Allow to extension remove only its hooks*/ && g_HooksGamerules[i].Element == iElement && g_HooksGamerules[i].iCallbackType == iCallbackType && !strcmp(g_HooksGamerules[i].pVar->GetName(), pProp) && pCallback == g_HooksGamerules[i].pCallback)
		{
			g_SendProxyManager.UnhookProxyGamerules(i);
			UnhookExtensionUnload(pExt);
			return true;
		}
	return false;
}

bool SendProxyManagerInterfaceImpl::AddUnhookListenerArray(IExtension * pExt, SendProp * pProp, CBaseEntity * pEntity, int iElement, CallBackType iCallbackType, void * pCallback, ISendProxyUnhookListener * pListener)
{
	const char * pPropName = pProp->GetName();
	return AddUnhookListenerArray(pExt, pPropName, pEntity, iElement, iCallbackType, pCallback, pListener);
}

bool SendProxyManagerInterfaceImpl::AddUnhookListenerArray(IExtension * pExt, const char * pProp, CBaseEntity * pEntity, int iElement, CallBackType iCallbackType, void * pCallback, ISendProxyUnhookListener * pListener)
{
	if (!pProp || !*pProp)
		return false;
	edict_t * pEdict = gameents->BaseEntityToEdict(pEntity);
	for (int i = 0; i < g_Hooks.Count(); i++)
		if (pEdict == g_Hooks[i].pEnt && g_Hooks[i].iCallbackType == iCallbackType && g_Hooks[i].Element == iElement && !strcmp(g_Hooks[i].pVar->GetName(), pProp) && pCallback == g_Hooks[i].pCallback)
		{
			ListenerCallbackInfo info;
			info.m_pExt = pExt;
			info.m_pExtAPI = pExt->GetAPI();
			info.m_pCallBack = pListener;
			HookExtensionUnload(pExt);
			g_Hooks[i].vListeners.AddToTail(info);
			return true;
		}
	return false;
}

bool SendProxyManagerInterfaceImpl::AddUnhookListenerArrayGamerules(IExtension * pExt, SendProp * pProp, int iElement, CallBackType iCallbackType, void * pCallback, ISendProxyUnhookListener * pListener)
{
	const char * pPropName = pProp->GetName();
	return AddUnhookListenerArrayGamerules(pExt, pPropName, iElement, iCallbackType, pCallback, pListener);
}

bool SendProxyManagerInterfaceImpl::AddUnhookListenerArrayGamerules(IExtension * pExt, const char * pProp, int iElement, CallBackType iCallbackType, void * pCallback, ISendProxyUnhookListener * pListener)
{
	if (!pProp || !*pProp)
		return false;
	for (int i = 0; i < g_HooksGamerules.Count(); i++)
		if (g_HooksGamerules[i].iCallbackType == iCallbackType && g_HooksGamerules[i].Element == iElement && !strcmp(g_HooksGamerules[i].pVar->GetName(), pProp) && pCallback == g_HooksGamerules[i].pCallback)
		{
			ListenerCallbackInfo info;
			info.m_pExt = pExt;
			info.m_pExtAPI = pExt->GetAPI();
			info.m_pCallBack = pListener;
			HookExtensionUnload(pExt);
			g_HooksGamerules[i].vListeners.AddToTail(info);
			return true;
		}
	return false;
}

bool SendProxyManagerInterfaceImpl::RemoveUnhookListenerArray(IExtension * pExt, SendProp * pProp, CBaseEntity * pEntity, int iElement, CallBackType iCallbackType, void * pCallback, ISendProxyUnhookListener * pListener)
{
	const char * pPropName = pProp->GetName();
	return RemoveUnhookListenerArray(pExt, pPropName, pEntity, iElement, iCallbackType, pCallback, pListener);
}

bool SendProxyManagerInterfaceImpl::RemoveUnhookListenerArray(IExtension * pExt, const char * pProp, CBaseEntity * pEntity, int iElement, CallBackType iCallbackType, void * pCallback, ISendProxyUnhookListener * pListener)
{
	if (!pProp || !*pProp)
		return false;
	
	edict_t * pEdict = gameents->BaseEntityToEdict(pEntity);
	for (int i = 0; i < g_Hooks.Count(); i++)
		if (g_Hooks[i].pEnt == pEdict && g_Hooks[i].iCallbackType == iCallbackType && g_Hooks[i].Element == iElement && !strcmp(g_Hooks[i].pVar->GetName(), pProp) && pCallback == g_Hooks[i].pCallback)
		{
			for (int j = 0; j < g_Hooks[i].vListeners.Count(); j++)
			{
				ListenerCallbackInfo info = g_Hooks[i].vListeners[j];
				if (info.m_pExt == pExt && info.m_pCallBack == pListener)
				{
					g_Hooks[i].vListeners.Remove(j);
					UnhookExtensionUnload(pExt);
					return true;
				}
			}
		}
	return false;
}

bool SendProxyManagerInterfaceImpl::RemoveUnhookListenerArrayGamerules(IExtension * pExt, SendProp * pProp, int iElement, CallBackType iCallbackType, void * pCallback, ISendProxyUnhookListener * pListener)
{
	const char * pPropName = pProp->GetName();
	return RemoveUnhookListenerArrayGamerules(pExt, pPropName, iElement, iCallbackType, pCallback, pListener);
}

bool SendProxyManagerInterfaceImpl::RemoveUnhookListenerArrayGamerules(IExtension * pExt, const char * pProp, int iElement, CallBackType iCallbackType, void * pCallback, ISendProxyUnhookListener * pListener)
{
	if (!pProp || !*pProp)
		return false;
	
	for (int i = 0; i < g_HooksGamerules.Count(); i++)
		if (g_HooksGamerules[i].iCallbackType == iCallbackType && g_HooksGamerules[i].Element == iElement && !strcmp(g_HooksGamerules[i].pVar->GetName(), pProp) && pCallback == g_HooksGamerules[i].pCallback)
		{
			for (int j = 0; j < g_HooksGamerules[i].vListeners.Count(); j++)
			{
				ListenerCallbackInfo info = g_HooksGamerules[i].vListeners[j];
				if (info.m_pExt == pExt && info.m_pCallBack == pListener)
				{
					g_HooksGamerules[i].vListeners.Remove(j);
					UnhookExtensionUnload(pExt);
					return true;
				}
			}
		}
	return false;
}

bool SendProxyManagerInterfaceImpl::IsProxyHooked(SendProp * pProp, CBaseEntity * pEntity)
{
	return IsProxyHooked(pProp->GetName(), pEntity);
}

bool SendProxyManagerInterfaceImpl::IsProxyHooked(const char * pProp, CBaseEntity * pEntity)
{
	if (!pProp || !*pProp)
		return false;
	edict_t * pEdict = gameents->BaseEntityToEdict(pEntity);
	for (int i = 0; i < g_Hooks.Count(); i++)
		if (g_Hooks[i].pEnt == pEdict && !strcmp(pProp, g_Hooks[i].pVar->GetName()))
			return true;
	return false;
}

bool SendProxyManagerInterfaceImpl::IsProxyHookedGamerules(SendProp * pProp)
{
	return IsProxyHookedGamerules(pProp->GetName());
}

bool SendProxyManagerInterfaceImpl::IsProxyHookedGamerules(const char * pProp)
{
	if (!pProp || !*pProp)
		return false;
	for (int i = 0; i < g_HooksGamerules.Count(); i++)
		if (!strcmp(pProp, g_HooksGamerules[i].pVar->GetName()))
			return true;
	return false;
}

bool SendProxyManagerInterfaceImpl::IsProxyHookedArray(SendProp * pProp, CBaseEntity * pEntity, int iElement)
{
	return IsProxyHookedArray(pProp->GetName(), pEntity, iElement);
}

bool SendProxyManagerInterfaceImpl::IsProxyHookedArray(const char * pProp, CBaseEntity * pEntity, int iElement)
{
	if (!pProp || !*pProp)
		return false;
	edict_t * pEdict = gameents->BaseEntityToEdict(pEntity);
	for (int i = 0; i < g_Hooks.Count(); i++)
		if (g_Hooks[i].pEnt == pEdict && g_Hooks[i].Element == iElement && !strcmp(pProp, g_Hooks[i].pVar->GetName()))
			return true;
	return false;
}

bool SendProxyManagerInterfaceImpl::IsProxyHookedArrayGamerules(SendProp * pProp, int iElement)
{
	return IsProxyHookedArrayGamerules(pProp->GetName(), iElement);
}

bool SendProxyManagerInterfaceImpl::IsProxyHookedArrayGamerules(const char * pProp, int iElement)
{
	if (!pProp || !*pProp)
		return false;
	for (int i = 0; i < g_HooksGamerules.Count(); i++)
		if (g_HooksGamerules[i].Element == iElement && !strcmp(pProp, g_HooksGamerules[i].pVar->GetName()))
			return true;
	return false;
}